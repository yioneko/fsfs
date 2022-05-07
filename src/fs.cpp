#include "fs.h"
#include "config.h"
#include "disk.h"
#include "fd_iter.h"
#include "parts/bitmap.h"
#include "parts/dirent.h"
#include "parts/inode.h"
#include "parts/super_block.h"
#include "utils.h"
#include <algorithm>
#include <fuse3/fuse_opt.h>
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

Dirent FS::get_dirent(const std::string &path) const {
  const auto path_parts = split_path(path);
  if (path_parts.empty()) {
    throw std::invalid_argument("root directory has no dirent");
  }

  auto cur_inode = this->get_inode(ROOT_INODE_NUM);
  auto cur_dir = this->get_dir_data(ROOT_INODE_NUM);
  for (auto i = 0; i < path_parts.size() - 1; ++i) {
    const auto dirent = cur_dir.find_entry(path_parts[i]);
    const auto inode = this->get_inode(dirent.inode_num);
    cur_dir =
        Dir::read_from_data(file_data_cbegin(inode), file_data_cend(inode));
  }

  return cur_dir.find_entry(path_parts.back());
}

Inode FS::get_inode(i_num_t inode_num) const {
  return Inode::read_from_disk(this->disk, get_inode_address(inode_num));
}

Dir FS::get_dir_data(i_num_t inode_num) const {
  const auto inode = this->get_inode(inode_num);
  return Dir::read_from_data(file_data_cbegin(inode), file_data_cend(inode));
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

FileDataConstIterator FS::file_data_cbegin(const Inode &inode) const {
  return FileDataConstIterator(*this, inode, 0, 0, false, 0);
}

FileDataConstIterator FS::file_data_cend(const Inode &inode) const {
  auto blk_num = inode.size / BLOCK_SIZE;
  const auto blk_offst = inode.size % BLOCK_SIZE;
  if (blk_num < INODE_DIRECT_ADDRESS_NUM) {
    return FileDataConstIterator(*this, inode, blk_num, 0, true, 0);
  } else {
    blk_num -= INODE_DIRECT_ADDRESS_NUM;
    return FileDataConstIterator(
        *this, inode, blk_num / INODE_INDIRECT_BLOCK_ADDRESS_NUM,
        blk_num % INODE_INDIRECT_BLOCK_ADDRESS_NUM, false, 0);
  }
}

blk_num_t FS::alloc_block() {
  const auto blk_num = this->bitmap.get_free_block();
  this->bitmap.blocks_bitmap.set(blk_num);
  this->sb.used_blocks++;
  // TODO: commit changes to disk
  return blk_num;
}
void FS::free_block(blk_num_t blk_num) {
  this->bitmap.blocks_bitmap.reset(blk_num);
  this->sb.used_blocks--;
  const auto blk_addr = this->disk.begin() + get_data_block_address(blk_num);
  std::fill(blk_addr, blk_addr + BLOCK_SIZE, 0);
}

i_num_t FS::alloc_inode() {
  const auto inode_num = this->bitmap.get_free_inode();
  this->bitmap.inodes_bitmap.set(inode_num);
  this->sb.used_inodes++;
  return inode_num;
}
void FS::free_inode(i_num_t inode_num) {
  this->bitmap.inodes_bitmap.reset(inode_num);
  this->sb.used_inodes--;
  const auto inode_addr = this->disk.begin() + get_inode_address(inode_num);
  std::fill(inode_addr, inode_addr + INODE_SIZE, 0);
}

void FS::free_inode_and_blocks(i_num_t inode_num) {
  const auto inode = this->get_inode(inode_num);
  this->free_inode(inode_num);
  for (auto blk_num : inode.get_refer_blk_nums()) {
    this->free_block(blk_num);
  }
}
