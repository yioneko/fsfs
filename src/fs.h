#ifndef FS_H
#define FS_H

#include "config.h"
#include "disk.h"
#include "parts/bitmap.h"
#include "parts/dirent.h"
#include "parts/inode.h"
#include "parts/super_block.h"
#include <climits>
#include <cstring>
#include <iterator>
#include <memory>

constexpr i_mode_t ROOT_DIR_MODE = 0555;
constexpr i_num_t ROOT_INODE_NUM = 0;

class FileDataIterator;
class FileDataConstIterator;

class FS {
  friend class FileDataIterator;
  friend class FileDataConstIterator;

public:
  FS(i_uid_t uid, i_gid_t gid);
  FS(const std::string &disk_file_path);

  void dump(const std::string &file_path) const;

  Dirent get_dirent(const std::string &path) const;
  Inode get_inode(i_num_t inode_num) const;
  Dir get_dir_data(i_num_t inode_num) const;

  void write_inode(const Inode &inode, i_num_t inode_num);
  template <typename Iter>
  i_fsize_t write_data(Iter begin, Iter end, Inode &inode,
                       i_fsize_t offset = 0);

  FileDataIterator file_data_begin(Inode &inode);
  FileDataIterator file_data_end(Inode &inode);
  FileDataConstIterator file_data_cbegin(const Inode &inode) const;
  FileDataConstIterator file_data_cend(const Inode &inode) const;

  blk_num_t alloc_block();
  void free_block(blk_num_t blk_num);

  i_num_t alloc_inode();
  void free_inode(i_num_t inode_num);

  void free_inode_and_blocks(i_num_t inode_num);

  SuperBlock sb;

private:
  Disk disk;
  Bitmap bitmap;

  void init_fs_on_disk(i_uid_t uid, i_gid_t gid);
};

#endif /* FS_H */
