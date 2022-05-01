#include "dirent.h"
#include "../utils.h"
#include <string>

dent_size_t Dirent::min_entry_size(const std::string &fname) {
  return sizeof(dent_size_t) + sizeof(i_num_t) + fname.size();
}

Dirent &&Dirent::read_from_iter(FileDataIterator &iter) {
  const auto entry_size = read_n<dent_size_t>(iter);
  const auto inum = read_n<i_num_t>(iter);

  auto fname = std::string();
  while (*iter != '\0') {
    fname += *iter;
    ++iter;
  }
  iter += entry_size - Dirent::min_entry_size(fname);

  return std::move(Dirent(entry_size, inum, fname));
}

std::vector<byte> &&Dirent::to_bytes() const {
  std::vector<byte> bytes(this->entry_size);
  std::fill(bytes.begin(), bytes.end(), 0);

  auto iter = bytes.begin();
  write_n(iter, this->entry_size);
  write_n(iter, this->inode_num);
  for (const auto &c : this->fname) {
    *iter = c;
    ++iter;
  }

  return std::move(bytes);
}

Dir::Dir(i_num_t self_inode_num, i_num_t parent_inode_num) {
  this->dirents.push_back(
      Dirent(Dirent::min_entry_size("."), self_inode_num, "."));
  this->dirents.push_back(
      Dirent(Dirent::min_entry_size(".."), parent_inode_num, ".."));
}

Dir &&Dir::read_from_data(FileDataIterator data_begin,
                          FileDataIterator data_end) {
  Dir dir;
  while (data_begin != data_end) {
    dir.dirents.push_back(Dirent::read_from_iter(data_begin));
  }
  return std::move(dir);
}

std::vector<byte> &&Dir::to_bytes() const {
  std::vector<byte> bytes;
  for (const auto &dirent : dirents) {
    auto dirent_bytes = dirent.to_bytes();
    bytes.insert(bytes.end(), dirent_bytes.begin(), dirent_bytes.end());
  }
  return std::move(bytes);
}
