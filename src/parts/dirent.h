#ifndef DIRENT_H
#define DIRENT_H

#include "../config.h"
#include "../disk.h"
#include "../utils.h"
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

  std::vector<byte> to_bytes() const;

  template <typename Iter> static Dirent read_from_iter(Iter &iter) {
    const auto entry_size = read_n<dent_size_t>(iter);
    const auto inum = read_n<i_num_t>(iter);

    auto fname = std::string();
    while (*iter != '\0') {
      fname += *iter;
      ++iter;
    }

    // skip padding
    iter += entry_size - sizeof(dent_size_t) - sizeof(i_num_t) - fname.size();

    return Dirent(entry_size, inum, fname);
  }
  static dent_size_t min_entry_size(const std::string &fname);
};

struct Dir {
  std::list<Dirent> dirents;

  // TODO: return iterator for lazy transforming
  std::vector<byte> to_bytes() const;

  Dir() = default;
  explicit Dir(i_num_t self_inode_num, i_num_t parent_inode_num);

  template <typename Iter>
  static Dir read_from_data(Iter data_begin, Iter data_end) {
    Dir dir;
    while (data_begin != data_end) {
      dir.dirents.push_back(Dirent::read_from_iter(data_begin));
    }
    return dir;
  }

  void add_entry(const std::string &fname, i_num_t inode_num);
  Dirent &find_entry(const std::string &fname);
  Dirent remove_entry(const std::string &fname);

  i_fsize_t size() const;
};

#endif /* DIRENT_H */
