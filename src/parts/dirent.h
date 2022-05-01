#ifndef DIRENT_H
#define DIRENT_H

#include "../config.h"
#include "../disk.h"
#include "../fs.h"
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

  static Dirent &&read_from_iter(FileDataIterator &iter);
  static dent_size_t min_entry_size(const std::string &fname);
};

struct Dir {
  std::list<Dirent> dirents;

  // TODO: return iterator for lazy transforming
  std::vector<byte> &&to_bytes() const;

  Dir() = default;
  explicit Dir(i_num_t self_inode_num, i_num_t parent_inode_num);

  static Dir &&read_from_data(FileDataIterator data_begin,
                              FileDataIterator data_end);

  void add_entry(const std::string &fname, i_num_t inode_num);
  Dirent &&remove_entry(const std::string &fname);
};

#endif /* DIRENT_H */
