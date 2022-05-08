#ifndef FS_H
#define FS_H

#include "config.h"
#include "disk.h"
#include "fd_iter.h"
#include "parts/bitmap.h"
#include "parts/dirent.h"
#include "parts/inode.h"
#include "parts/super_block.h"
#include <climits>
#include <cstring>
#include <iterator>
#include <memory>
#include <sys/stat.h>

constexpr i_mode_t ROOT_DIR_MODE = S_IFDIR | 0775;
constexpr i_num_t ROOT_INODE_NUM = 0;

class FS {
  friend class FileDataIterator;
  friend class FileDataConstIterator;

public:
  FS(i_uid_t uid, i_gid_t gid);
  FS(const std::string &disk_file_path);

  void dump(const std::string &file_path);

  Dirent get_dirent(const std::string &path) const;
  Inode get_inode(i_num_t inode_num) const;
  Dir get_dir_data(i_num_t inode_num) const;

  void write_inode(const Inode &inode, i_num_t inode_num);

  // This updates inode as well
  template <typename Iter>
  i_fsize_t write_data(Iter data_begin, Iter data_end, Inode &inode,
                       i_fsize_t offset = 0) {
    auto file_data_iter = this->file_data_begin(inode);
    for (auto i = 0; i < offset; ++i) {
      ++file_data_iter;
    }

    auto write_bytes = 0;
    for (auto data_iter = data_begin; data_iter != data_end;
         ++data_iter, ++file_data_iter, ++write_bytes) {
      *file_data_iter = *data_iter;
    }
    inode.size = std::max(inode.size, offset + write_bytes);
    return write_bytes;
  }

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
