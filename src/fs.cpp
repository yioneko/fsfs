#include "fs.h"
#include "config.h"
#include "disk.h"
#include "parts/bitmap.h"
#include "parts/dirent.h"
#include "parts/inode.h"
#include "parts/super_block.h"
#include <algorithm>
#include <iterator>
#include <string>

FS::FS(i_uid_t uid, i_gid_t gid) { this->init_fs_on_disk(uid, gid); }

FS::FS(const std::string &disk_file_path)
    : disk(Disk::load(disk_file_path)),
      sb(SuperBlock::read_from_disk(this->disk)),
      bitmap(Bitmap::read_from_disk(this->disk)) {}

void FS::dump(const std::string &file_path) const {
  this->disk.save(file_path);
}

void FS::init_fs_on_disk(i_uid_t uid, i_gid_t gid) {
  auto root_inode = Inode(ROOT_DIR_MODE, uid, gid);
  auto root_dir = Dir(ROOT_INODE_NUM, ROOT_INODE_NUM);
  const auto root_dir_bytes = root_dir.to_bytes();

  this->bitmap.inodes_bitmap.set(ROOT_INODE_NUM);
  this->write_inode(root_inode, ROOT_INODE_NUM);
  this->write_data(root_dir_bytes.begin(), root_dir_bytes.end(), root_inode);
}

void FS::write_inode(const Inode &inode, i_num_t inode_num) {
  const auto inode_bytes = inode.to_bytes();
  std::move(inode_bytes.first.begin(), inode_bytes.first.end(),
            this->disk.begin() + get_inode_address(inode_num));

  for (const auto &indirect_address : inode_bytes.second) {
    std::move(indirect_address.second.begin(), indirect_address.second.end(),
              this->disk.begin() +
                  get_data_block_address(indirect_address.first));
  }
}

template <typename Iter>
void FS::write_data(Iter data_begin, Iter data_end, Inode &inode,
                    size_t offset) {
  auto file_data_iter = this->file_data_begin(inode);
  for (auto i = 0; i < offset; ++i) {
    ++file_data_iter;
  }
  for (auto data_iter = data_begin; data_iter != data_end;
       ++data_iter, ++file_data_iter) {
    *file_data_iter = *data_iter;
  }
}

FileDataIterator FS::file_data_begin(Inode &inode) {
  return FileDataIterator(*this, inode, 0, 0, false, 0);
}

FileDataIterator FS::file_data_end(Inode &inode) {
  auto blk_num = inode.size / BLOCK_SIZE;
  const auto blk_offst = inode.size % BLOCK_SIZE;
  if (blk_num < INODE_DIRECT_ADDRESS_NUM) {
    return FileDataIterator(*this, inode, blk_num, 0, true, 0);
  } else {
    blk_num -= INODE_DIRECT_ADDRESS_NUM;
    return FileDataIterator(
        *this, inode, blk_num / INODE_INDIRECT_BLOCK_ADDRESS_NUM,
        blk_num % INODE_INDIRECT_BLOCK_ADDRESS_NUM, false, 0);
  }
}

blk_num_t FS::alloc_block() {
  const auto blk_num = this->bitmap.get_free_block();
  this->bitmap.blocks_bitmap.set(blk_num);
  return blk_num;
}
void FS::free_block(blk_num_t blk_num) {
  this->bitmap.blocks_bitmap.reset(blk_num);
  const auto blk_addr = this->disk.begin() + get_data_block_address(blk_num);
  std::fill(blk_addr, blk_addr + BLOCK_SIZE, 0);
}

i_num_t FS::alloc_inode() {
  const auto inode_num = this->bitmap.get_free_inode();
  this->bitmap.inodes_bitmap.set(inode_num);
  return inode_num;
}
void FS::free_inode(i_num_t inode_num) {
  this->bitmap.inodes_bitmap.reset(inode_num);
  const auto inode_addr = this->disk.begin() + get_inode_address(inode_num);
  std::fill(inode_addr, inode_addr + INODE_SIZE, 0);
}
