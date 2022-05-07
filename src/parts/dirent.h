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
  i_num_t inode_num;
  std::string fname;

  Dirent(dent_size_t entry_size, i_num_t inode_num, std::string fname)
      : entry_size(entry_size), inode_num(inode_num), fname(fname) {}

  std::vector<byte> &&to_bytes() const;

  template <typename Iter> static Dirent &&read_from_iter(Iter &iter);
  static dent_size_t min_entry_size(const std::string &fname);
};

struct Dir {
  std::list<Dirent> dirents;

  // TODO: return iterator for lazy transforming
  std::vector<byte> &&to_bytes() const;

  Dir() = default;
  explicit Dir(i_num_t self_inode_num, i_num_t parent_inode_num);

  template <typename Iter>
  static Dir &&read_from_data(Iter data_begin, Iter data_end);

  void add_entry(const std::string &fname, i_num_t inode_num);
  Dirent &find_entry(const std::string &fname);
  Dirent &&remove_entry(const std::string &fname);

  i_fsize_t size() const;
};

#endif /* DIRENT_H */
