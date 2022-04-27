#include "fs.h"
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

void FS::init_fs_on_disk(i_uid_t uid, i_gid_t gid) {
  auto root_inode = Inode(ROOT_DIR_MODE, uid, gid);
  auto root_dir = Dir(ROOT_INODE_NUM, ROOT_INODE_NUM);

  const auto inode_bytes = root_inode.to_bytes();
  std::move(inode_bytes.begin(), inode_bytes.end(),
            this->disk.begin() + this->get_inode_address(ROOT_INODE_NUM));
  this->bitmap.inodes_bitmap.set(ROOT_INODE_NUM);
}

template <typename Iter>
void FS::write_data(Iter begin, Iter end, const Inode &inode, size_t offset) {
  auto cur_block_num = offset / BLOCK_SIZE;
}
