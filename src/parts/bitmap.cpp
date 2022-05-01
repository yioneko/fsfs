#include "bitmap.h"
#include <climits>
#include <stdexcept>

Bitmap &&Bitmap::read_from_disk(const Disk &disk) {
  Bitmap bitmap;

  auto disk_iter = disk.cbegin() + INODES_BITMAP_START;
  for (auto i = 0; i < INODES_BITMAP_SIZE / CHAR_BIT; i++) {
    const auto byte = *disk_iter++;
    for (auto j = 0; j < CHAR_BIT; j++) {
      bitmap.inodes_bitmap.set(i * CHAR_BIT + j, byte & (1 << j));
    }
  }

  disk_iter = disk.cbegin() + BLOCKS_BITMAP_START;
  for (auto i = 0; i < BLOCKS_BITMAP_SIZE / CHAR_BIT; i++) {
    const auto byte = *(disk_iter++);
    for (auto j = 0; j < CHAR_BIT; j++) {
      bitmap.blocks_bitmap.set(i * CHAR_BIT + j, byte & (1 << j));
    }
  }

  return std::move(bitmap);
}

i_num_t Bitmap::get_free_inode(i_num_t hint) {
  if (!this->inodes_bitmap[hint]) {
    return hint;
  }
  for (auto i = (hint + 1) % this->inodes_bitmap.size(); i < hint;
       i = (i + 1) % this->inodes_bitmap.size()) {
    if (!this->inodes_bitmap[i]) {
      return i;
    }
  }

  throw std::runtime_error("No free inodes");
}

blk_num_t Bitmap::get_free_block(blk_num_t hint) {
  if (!this->blocks_bitmap[hint]) {
    return hint;
  }
  for (auto i = (hint + 1) % this->blocks_bitmap.size(); i < hint;
       i = (i + 1) % this->blocks_bitmap.size()) {
    if (!this->blocks_bitmap[i]) {
      return i;
    }
  }

  throw new std::runtime_error("No free blocks");
}

std::array<byte, INODES_BITMAP_SIZE / CHAR_BIT> &&
Bitmap::inodes_bitmap_bytes() const {
  std::array<byte, INODES_BITMAP_SIZE / CHAR_BIT> bytes;
  for (auto i = 0; i < INODES_BITMAP_SIZE / CHAR_BIT; i++) {
    byte byte = 0;
    for (auto j = 0; j < CHAR_BIT; j++) {
      byte |= this->inodes_bitmap[i * CHAR_BIT + j] << j;
    }
    bytes[i] = byte;
  }
  return std::move(bytes);
}

std::array<byte, BLOCKS_BITMAP_SIZE / CHAR_BIT> &&
Bitmap::blocks_bitmap_bytes() const {
  std::array<byte, BLOCKS_BITMAP_SIZE / CHAR_BIT> bytes;
  for (auto i = 0; i < BLOCKS_BITMAP_SIZE / CHAR_BIT; i++) {
    byte byte = 0;
    for (auto j = 0; j < CHAR_BIT; j++) {
      byte |= this->blocks_bitmap[i * CHAR_BIT + j] << j;
    }
    bytes[i] = byte;
  }
  return std::move(bytes);
}
