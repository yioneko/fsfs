#ifndef SUPER_BLOCK_H
#define SUPER_BLOCK_H

#include "../config.h"
#include "../disk.h"

class SuperBlock {
  friend class FS;

  SuperBlock() : used_blocks(0), used_inodes(0) {}

public:
  sb_used_b_t used_blocks;
  sb_used_i_t used_inodes;
  static SuperBlock &&read_from_disk(const Disk &);
  std::array<byte, SUPER_BLOCK_SIZE> &&to_bytes() const;
};

#endif /* SUPER_BLOCK_H */
