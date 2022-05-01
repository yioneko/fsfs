#ifndef CONFIG_H
#define CONFIG_H

#include <climits>
#include <cmath>
#include <cstddef>

#define TYPE_BITS(t) (sizeof(t) * CHAR_BIT)
using std::size_t;

// The unit for size is byte if unspecified.

// Basic config for disk space
constexpr size_t ADDRESS_LENGTH = 24;
constexpr size_t ADDRESS_SIZE = 24 / CHAR_BIT;
constexpr size_t DISK_SIZE =
    1 << ADDRESS_LENGTH; // assume the space is byte addressable
constexpr size_t BLOCK_SIZE = 1 << 10;
typedef unsigned short blk_num_t;
constexpr size_t BLOCK_NUM =
    18 * (1 << 10); // ~= (DISK_SIZE - INODE_NUM * INODE_SIZE) / BLOCK_SIZE

// inode
/* Unit: byte
+----+----+----+----+----+----+----+----+
|       MODE        |   UID   |   GID   |
+----+----+----+----+----+----+----+----+
|       SIZE        |    ACCESS TIME    |
+----+----+----+----+----+----+----+----+
|    MODIFY TIME    |                   |
+----+----+----+----+                   |
|            10x DIRECT ADDRS           |
|                                       |
+----+----+----+----+----+----+----+----+
|INDIRECT |                             |
+----+----+                             +
/         <PADDING to 64 bytes>         /
/                                       /
+----+----+----+----+----+----+----+----+
 */
typedef unsigned int i_mode_t;
typedef unsigned short i_uid_t;
typedef unsigned short i_gid_t;
typedef unsigned int i_fsize_t;
typedef unsigned int i_time_t;

typedef unsigned short i_num_t;
constexpr size_t INODES_NUM_MAX = 1 << TYPE_BITS(i_num_t);

constexpr size_t INODE_DIRECT_ADDRESS_NUM = 10;
constexpr size_t INODE_INDIRECT_ADDRESS_NUM = 1;
constexpr size_t INODE_INDIRECT_BLOCK_ADDRESS_NUM =
    BLOCK_SIZE / sizeof(blk_num_t);
constexpr size_t INODE_SIZE_WITHOUT_PADDING =
    sizeof(i_mode_t) + sizeof(i_uid_t) + sizeof(i_gid_t) + sizeof(i_fsize_t) +
    sizeof(i_time_t) * 2 + INODE_DIRECT_ADDRESS_NUM * sizeof(blk_num_t) +
    INODE_INDIRECT_ADDRESS_NUM * sizeof(blk_num_t);
constexpr size_t INODE_SIZE = 64;

// directory entry
/* Unit: byte
+----+----+----+----+----+----+----+----+
|ESIZE|  INUM  |                        |
+--+--+--+--+--+                        +
/              <FILE NAME>              /
/              <PADDING>                /
+----+----+----+----+----+----+----+----+
 */
typedef unsigned char dent_size_t;
constexpr size_t DIRENT_MAX_SIZE = (1 << TYPE_BITS(dent_size_t)) - 1;

// overall disk structure
/*
┌─────┬──────┬────────┬─────────────┬────────────────────┐
│super│inodes│ blocks │    inodes   │       blocks       │
│block│bitmap│ bitmap │             │                    │
└─────┴──────┴────────┴─────────────┴────────────────────┘
  */

// super block
/* Unit: byte
+----+----+----+----+
| INODES  | BLOCKS  |
+----+----+----+----+
 */
typedef unsigned short sb_used_i_t;
typedef unsigned short sb_used_b_t;
constexpr size_t SUPER_BLOCK_SIZE = sizeof(sb_used_i_t) + sizeof(sb_used_b_t);

constexpr size_t INODES_BITMAP_START = SUPER_BLOCK_SIZE;
constexpr size_t INODES_BITMAP_SIZE = INODES_NUM_MAX; // in bits

constexpr size_t BLOCKS_BITMAP_START = INODES_BITMAP_START + INODES_BITMAP_SIZE;
constexpr size_t BLOCKS_BITMAP_SIZE = BLOCK_NUM; // in bits

constexpr size_t INODES_START = BLOCKS_BITMAP_START + BLOCKS_BITMAP_SIZE;
constexpr size_t INODES_SIZE = INODES_NUM_MAX * INODE_SIZE;

constexpr size_t BLOCKS_START = INODES_START + INODES_SIZE;

inline auto get_inode_address(i_num_t inode_num) {
  return INODES_START + inode_num * INODE_SIZE;
}
inline auto get_data_block_address(blk_num_t block_num) {
  return BLOCKS_START + block_num * BLOCK_SIZE;
}

#endif /* CONFIG_H */
