#ifndef FD_ITER_H
#define FD_ITER_H

#include "disk.h"
#include <iterator>

class FS;
class Inode;

class FileDataIterator {
public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = byte;
  using difference_type = std::ptrdiff_t;
  using pointer = byte *;
  using reference = byte &;

  FileDataIterator(FS &fs, Inode &inode, size_t addr_index,
                   size_t indirect_addr_index, bool is_direct,
                   size_t blk_offset = 0)
      : fs(fs), inode(inode), addr_index(addr_index),
        indirect_addr_index(indirect_addr_index), is_direct(is_direct),
        blk_offset(blk_offset) {}

  FileDataIterator &operator++();
  FileDataIterator &operator+=(size_t);
  FileDataIterator operator+(size_t);
  value_type &operator*();
  bool operator==(const FileDataIterator &) const;
  bool operator!=(const FileDataIterator &other) const;

  blk_num_t &get_current_block_num();

private:
  FS &fs;
  Inode &inode;

  size_t indirect_addr_index;
  size_t addr_index;
  bool is_direct;
  size_t blk_offset;

  void allocate_if_needed();
};

class FileDataConstIterator {
public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = const byte;
  using difference_type = std::ptrdiff_t;
  using pointer = byte *const;
  using reference = const byte &;

  FileDataConstIterator(const FS &fs, const Inode &inode, size_t addr_index,
                        size_t indirect_addr_index, bool is_direct,
                        size_t blk_offset = 0)
      : fs(fs), inode(inode), addr_index(addr_index),
        indirect_addr_index(indirect_addr_index), is_direct(is_direct),
        blk_offset(blk_offset) {}

  FileDataConstIterator &operator++();
  FileDataConstIterator &operator+=(size_t);
  FileDataConstIterator operator+(size_t);
  value_type &operator*();
  bool operator==(const FileDataConstIterator &other) const;
  bool operator!=(const FileDataConstIterator &other) const;

  blk_num_t get_current_block_num();

private:
  const FS &fs;
  const Inode &inode;

  size_t indirect_addr_index;
  size_t addr_index;
  bool is_direct;
  size_t blk_offset;
};

#endif /* FD_ITER_H */
