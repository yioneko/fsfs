#ifndef DIRENT_H
#define DIRENT_H

#include "../config.h"
#include "../disk.h"
#include <array>
#include <list>
#include <string>
#include <vector>

struct Dirent {
  dent_size_t entry_size;
  dent_fname_size_t fname_size;
  i_num_t inode_num;
  std::string fname;

  std::vector<byte> &&to_bytes() const;

  static Dirent &&read_from_disk(const Disk &, const size_t offset);
  static dent_size_t min_entry_size(const std::string &fname);
};

struct Dir {
  i_num_t self_inode_num;
  i_num_t parent_inode_num;
  std::list<Dirent> dirents;

  // TODO: return iterator for lazy transforming
  std::vector<byte> &&to_bytes() const;

  explicit Dir(i_num_t self_inode_num, i_num_t parent_inode_num);
};

#endif /* DIRENT_H */
