#ifndef FS_H
#define FS_H

#include "config.h"
#include "disk.h"
#include "parts/bitmap.h"
#include "parts/inode.h"
#include "parts/super_block.h"
#include <iterator>

constexpr i_mode_t ROOT_DIR_MODE = 0555;
constexpr i_num_t ROOT_INODE_NUM = 0;

class FS {
public:
  FS(i_uid_t uid, i_gid_t gid);
  FS(const std::string &disk_file_path);

private:
  Disk disk;
  SuperBlock sb;
  Bitmap bitmap;

  void init_fs_on_disk(i_uid_t uid, i_gid_t gid);

  size_t get_inode_address(i_num_t inode_num) const;
  size_t get_data_block_address(blk_num_t block_num) const;

  i_num_t write_inode(const Inode &inode);
  template <typename Iter>
  void write_data(Iter begin, Iter end, const Inode &inode, size_t offset = 0);
};

#endif /* FS_H */
