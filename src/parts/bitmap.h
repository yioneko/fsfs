#ifndef BITMAP_H
#define BITMAP_H

#include "../config.h"
#include "../disk.h"
#include <array>
#include <bitset>
#include <climits>

class Bitmap {
  friend class FS;

  std::bitset<INODES_BITMAP_SIZE> inodes_bitmap;
  std::bitset<BLOCKS_BITMAP_SIZE> blocks_bitmap;
  Bitmap();

public:
  i_num_t get_free_inode(i_num_t hint = 0);
  blk_num_t get_free_block(blk_num_t hint = 0);

  static Bitmap &&read_from_disk(const Disk &);
  std::array<byte, INODES_BITMAP_SIZE / CHAR_BIT> &&inodes_bitmap_bytes() const;
  std::array<byte, BLOCKS_BITMAP_SIZE / CHAR_BIT> &&blocks_bitmap_bytes() const;
};

#endif /* BITMAP_H */
