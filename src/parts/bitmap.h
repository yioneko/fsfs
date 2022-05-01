#ifndef BITMAP_H
#define BITMAP_H

#include "../config.h"
#include "../disk.h"
#include <bitset>

class Bitmap {
  friend class FS;

  std::bitset<INODES_BITMAP_SIZE> inodes_bitmap;
  std::bitset<BLOCKS_BITMAP_SIZE> blocks_bitmap;
  Bitmap();

public:
  i_num_t get_free_inode(i_num_t hint = 0, bool no_set = false);
  blk_num_t get_free_block(blk_num_t hint = 0, bool no_set = false);

  static Bitmap &&read_from_disk(const Disk &);
};

#endif /* BITMAP_H */
