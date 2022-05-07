#include "dirent.h"
#include "../utils.h"
#include <algorithm>
#include <stdexcept>
#include <string>

dent_size_t Dirent::min_entry_size(const std::string &fname) {
  return sizeof(dent_size_t) + sizeof(i_num_t) + fname.size();
}

template <typename Iter> Dirent &&Dirent::read_from_iter(Iter &iter) {
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

template <typename Iter>
Dir &&Dir::read_from_data(Iter data_begin, Iter data_end) {
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

void Dir::add_entry(const std::string &fname, i_num_t inode_num) {
  const auto min_size = Dirent::min_entry_size(fname);
  auto dirent_slot = std::find_if(
      this->dirents.begin(), this->dirents.end(), [&](const Dirent &d) {
        return d.entry_size - Dirent::min_entry_size(d.fname) >= min_size;
      });

  if (dirent_slot != this->dirents.end()) {
    const auto shrinked_size = Dirent::min_entry_size(dirent_slot->fname);
    const auto new_dirent =
        Dirent(dirent_slot->entry_size - shrinked_size, inode_num, fname);
    dirent_slot->entry_size = shrinked_size;
    this->dirents.insert(++dirent_slot, new_dirent);
  } else {
    const auto new_dirent = Dirent(min_size, inode_num, fname);
    this->dirents.push_back(new_dirent);
  }
}

Dirent &Dir::find_entry(const std::string &fname) {
  auto dirent = std::find_if(this->dirents.begin(), this->dirents.end(),
                             [&](const Dirent &d) { return d.fname == fname; });
  if (dirent == this->dirents.end()) {
    throw new std::runtime_error("Directory entry not found");
  }
  return *dirent;
}

Dirent &&Dir::remove_entry(const std::string &fname) {
  auto it = std::find_if(this->dirents.begin(), this->dirents.end(),
                         [&](const Dirent &d) { return d.fname == fname; });

  if (it == this->dirents.end()) {
    auto res = this->dirents.back();
    this->dirents.pop_back();
    return std::move(res);
  } else if (it == this->dirents.begin()) {
    auto res = *this->dirents.begin();
    *this->dirents.begin() = this->dirents.back();
    this->dirents.pop_back();
    return std::move(res);
  } else {
    auto res = *it;
    --it;
    it->entry_size += res.entry_size;
    this->dirents.erase(++it);
    return std::move(res);
  }
}

i_fsize_t Dir::size() const {
  i_fsize_t size = 0;
  for (const auto &dirent : this->dirents) {
    size += dirent.entry_size;
  }
  return size;
}
