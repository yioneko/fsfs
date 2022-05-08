#include "bitmap.h"
#include <climits>
#include <stdexcept>

Bitmap::Bitmap() {
  this->blocks_bitmap.reset();
  this->inodes_bitmap.reset();
}

Bitmap Bitmap::read_from_disk(const Disk &disk) {
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

  return bitmap;
}

i_num_t Bitmap::get_free_inode(i_num_t hint) {
  for (auto i = 0; i < this->inodes_bitmap.size(); i++) {
    if (!this->inodes_bitmap[(i + hint) % this->inodes_bitmap.size()]) {
      return i;
    }
  }

  throw std::runtime_error("No free blocks");
}

blk_num_t Bitmap::get_free_block(blk_num_t hint) {
  for (auto i = 0; i < this->blocks_bitmap.size(); i++) {
    if (!this->blocks_bitmap[(i + hint) % this->blocks_bitmap.size()]) {
      return i + 1; // '0' block num indicate empty block
    }
  }

  throw std::runtime_error("No free blocks");
}

std::array<byte, INODES_BITMAP_SIZE / CHAR_BIT>
Bitmap::inodes_bitmap_bytes() const {
  std::array<byte, INODES_BITMAP_SIZE / CHAR_BIT> bytes;
  for (auto i = 0; i < INODES_BITMAP_SIZE / CHAR_BIT; i++) {
    byte byte = 0;
    for (auto j = 0; j < CHAR_BIT; j++) {
      byte |= this->inodes_bitmap[i * CHAR_BIT + j] << j;
    }
    bytes[i] = byte;
  }
  return bytes;
}

std::array<byte, BLOCKS_BITMAP_SIZE / CHAR_BIT>
Bitmap::blocks_bitmap_bytes() const {
  std::array<byte, BLOCKS_BITMAP_SIZE / CHAR_BIT> bytes;
  for (auto i = 0; i < BLOCKS_BITMAP_SIZE / CHAR_BIT; i++) {
    byte byte = 0;
    for (auto j = 0; j < CHAR_BIT; j++) {
      byte |= this->blocks_bitmap[i * CHAR_BIT + j] << j;
    }
    bytes[i] = byte;
  }
  return bytes;
}
