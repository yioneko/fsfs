#include "fd_iter.h"
#include "fs.h"

// TODO: how to extract common logic of FileDataIterator and
// FileDataConstIterator?

FileDataIterator &FileDataIterator::operator++() {
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
FileDataIterator &FileDataIterator::operator+=(size_t n) {
  for (size_t i = 0; i < n; ++i) {
    ++(*this);
  }
  return *this;
}

FileDataIterator FileDataIterator::operator+(size_t n) {
  FileDataIterator tmp(*this);
  tmp += n;
  return tmp;
}

FileDataIterator::value_type &FileDataIterator::operator*() {
  allocate_if_needed();
  return *(fs.disk.begin() + get_data_block_address(get_current_block_num()) +
           blk_offset);
}

bool FileDataIterator::operator==(const FileDataIterator &other) const {
  return std::addressof(inode) == std::addressof(other.inode) &&
         addr_index == other.addr_index &&
         indirect_addr_index == other.indirect_addr_index &&
         is_direct == other.is_direct && blk_offset == other.blk_offset;
}

bool FileDataIterator::operator!=(const FileDataIterator &other) const {
  return !(*this == other);
}

blk_num_t FileDataIterator::get_current_block_num() {
  return is_direct
             ? inode.direct_addresses[addr_index]
             : inode.indirect_block_addresses[indirect_addr_index][addr_index];
}

void FileDataIterator::allocate_if_needed() {
  for (auto i = inode.indirect_block_addresses.size(); i < indirect_addr_index;
       ++i) {
    inode.expand_indirect_addresses({fs.alloc_block()});
  }
  auto cur_block_num = this->get_current_block_num();
  if (cur_block_num == 0) {
    cur_block_num = fs.alloc_block();
  }
}

FileDataConstIterator &FileDataConstIterator::operator++() {
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
  }
  return *this;
}

FileDataConstIterator &FileDataConstIterator::operator+=(size_t n) {
  for (size_t i = 0; i < n; ++i) {
    ++(*this);
  }
  return *this;
}

FileDataConstIterator FileDataConstIterator::operator+(size_t n) {
  FileDataConstIterator tmp(*this);
  tmp += n;
  return tmp;
}

FileDataConstIterator::value_type &FileDataConstIterator::operator*() {
  return *(fs.disk.cbegin() + get_data_block_address(get_current_block_num()) +
           blk_offset);
}

bool FileDataConstIterator::operator==(
    const FileDataConstIterator &other) const {
  return std::addressof(inode) == std::addressof(other.inode) &&
         addr_index == other.addr_index &&
         indirect_addr_index == other.indirect_addr_index &&
         is_direct == other.is_direct && blk_offset == other.blk_offset;
}

bool FileDataConstIterator::operator!=(
    const FileDataConstIterator &other) const {
  return !(*this == other);
}

blk_num_t FileDataConstIterator::get_current_block_num() {
  return is_direct
             ? inode.direct_addresses[addr_index]
             : inode.indirect_block_addresses[indirect_addr_index][addr_index];
}
