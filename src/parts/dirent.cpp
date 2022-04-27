#include "dirent.h"

dent_size_t Dirent::min_entry_size(const std::string &fname) {
  return sizeof(dent_size_t) + sizeof(dent_fname_size_t) + sizeof(i_num_t) +
         fname.size();
}

Dir::Dir(i_num_t self_inode_num, i_num_t parent_inode_num)
    : self_inode_num(self_inode_num), parent_inode_num(parent_inode_num) {
  this->dirents.push_back({.entry_size = Dirent::min_entry_size("."),
                           .fname_size = 1,
                           .inode_num = self_inode_num,
                           .fname = "."});
  this->dirents.push_back({.entry_size = Dirent::min_entry_size(".."),
                           .fname_size = 2,
                           .inode_num = parent_inode_num,
                           .fname = ".."});
}
