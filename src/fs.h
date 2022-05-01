#ifndef FS_H
#define FS_H

#include "config.h"
#include "disk.h"
#include "parts/bitmap.h"
#include "parts/inode.h"
#include "parts/super_block.h"
#include <climits>
#include <cstring>
#include <iterator>
#include <memory>

constexpr i_mode_t ROOT_DIR_MODE = 0555;
constexpr i_num_t ROOT_INODE_NUM = 0;

class FileDataIterator;

class FS {
  friend class FileDataIterator;

public:
  FS(i_uid_t uid, i_gid_t gid);
  FS(const std::string &disk_file_path);

  void dump(const std::string &file_path) const;

private:
  Disk disk;
  SuperBlock sb;
  Bitmap bitmap;

  void init_fs_on_disk(i_uid_t uid, i_gid_t gid);

  void write_inode(const Inode &inode, i_num_t inode_num);
  template <typename Iter>
  void write_data(Iter begin, Iter end, Inode &inode, size_t offset = 0);

  FileDataIterator file_data_begin(Inode &inode);
  FileDataIterator file_data_end(Inode &inode);

  blk_num_t alloc_block();
  void free_block(blk_num_t blk_num);

  i_num_t alloc_inode();
  void free_inode(i_num_t inode_num);
};

class FileDataIterator {
public:
  FileDataIterator(FS &fs, Inode &inode, size_t addr_index,
                   size_t indirect_addr_index, bool is_direct,
                   size_t blk_offset = 0)
      : fs(fs), inode(inode), addr_index(addr_index),
        indirect_addr_index(indirect_addr_index), is_direct(is_direct),
        blk_offset(blk_offset) {}

  FileDataIterator &operator++() {
    ++blk_offset;
    if (blk_offset == BLOCK_SIZE) {
      blk_offset = 0;
      ++addr_index;
      if (is_direct) {
        if (addr_index == INODE_DIRECT_ADDRESS_NUM) {
          is_direct = false;
          addr_index = 0;
        }
      } else if (addr_index == INODE_INDIRECT_BLOCK_ADDRESS_NUM) {
        addr_index = 0;
        ++indirect_addr_index;
      }
      allocate_if_needed(); // TODO: not standard but for convenience
    }
    return *this;
  }

  // TODO: need optimization
  FileDataIterator &operator+=(size_t n) {
    for (size_t i = 0; i < n; ++i) {
      ++(*this);
    }
    return *this;
  }
  FileDataIterator operator+(size_t n) {
    FileDataIterator tmp(*this);
    tmp += n;
    return tmp;
  }

  byte &operator*() {
    allocate_if_needed();
    return *(fs.disk.begin() + get_data_block_address(get_current_block_num()) +
             blk_offset);
  }

  bool operator==(const FileDataIterator &other) const {
    return std::addressof(inode) == std::addressof(other.inode) &&
           addr_index == other.addr_index &&
           indirect_addr_index == other.indirect_addr_index &&
           is_direct == other.is_direct && blk_offset == other.blk_offset;
  }

  bool operator!=(const FileDataIterator &other) const {
    return !(*this == other);
  }

  blk_num_t &get_current_block_num() {
    return is_direct
               ? inode.direct_addresses[addr_index]
               : inode
                     .indirect_block_addresses[indirect_addr_index][addr_index];
  }

private:
  FS &fs;
  Inode &inode;

  size_t indirect_addr_index;
  size_t addr_index;
  bool is_direct;
  size_t blk_offset;

  void allocate_if_needed() {
    for (auto i = inode.indirect_block_addresses.size();
         i < indirect_addr_index; ++i) {
      inode.expand_indirect_addresses({fs.alloc_block()});
    }
    auto cur_block_num = this->get_current_block_num();
    if (cur_block_num == 0) {
      cur_block_num = fs.alloc_block();
    }
  }
};

#endif /* FS_H */
