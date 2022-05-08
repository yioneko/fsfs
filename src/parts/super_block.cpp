#include "super_block.h"
#include "../utils.h"

SuperBlock SuperBlock::read_from_disk(const Disk &disk) {
  SuperBlock super_block;
  auto disk_iter = disk.cbegin();
  super_block.used_blocks = read_n<sb_used_b_t>(disk_iter);
  super_block.used_inodes = read_n<sb_used_i_t>(disk_iter);
  return super_block;
}

std::array<byte, SUPER_BLOCK_SIZE> SuperBlock::to_bytes() const {
  std::array<byte, SUPER_BLOCK_SIZE> bytes;
  auto bytes_iter = bytes.begin();
  write_n(bytes_iter, this->used_blocks);
  write_n(bytes_iter, this->used_inodes);
  return bytes;
}
