#include <climits>
#include <cmath>
#include <cstddef>

#define TYPE_BITS(t) (sizeof(t) * CHAR_BIT)
using std::size_t;

// The unit for size is byte if unspecified.

// Basic config for disk space
constexpr size_t ADDRESS_LENGTH = 24;
constexpr size_t DISK_SIZE =
    1 << ADDRESS_LENGTH; // assume the space is byte addressable
constexpr size_t BLOCK_SIZE = 1 << 10;
constexpr size_t BLOCK_NUM =
    18 * (1 << 10); // ~= (DISK_SIZE - INODE_NUM * INODE_SIZE) / BLOCK_SIZE

// inode
/* Unit: byte
+----+----+----+----+----+----+----+----+
|MODE|UID |GID |     SIZE     |  ACCESS
+----+----+----+----+----+----+----+----+
   TIME   |    MODIFY TIME    |         |
+----+----+----+----+----+----+         |
|                                       |
|         10x DIRECT ADDRESSES          |
|                                       |
|                   +----+----+----+----+
|                   |INDIRECT ADRESS|   |
+----+----+----+----+----+----+----+    |
/         <PADDING to 64 bytes>         /
/                                       /
+----+----+----+----+----+----+----+----+
 */
typedef unsigned char i_mode_t;
typedef unsigned char i_uid_t;
typedef unsigned char i_gid_t;
typedef unsigned int i_fsize_t;
typedef unsigned int i_time_t;

typedef unsigned short i_num_t;
constexpr size_t INODES_NUM_MAX = 1 << TYPE_BITS(i_num_t);

constexpr size_t INODE_DIRECT_ADDRESS_NUM = 10;
constexpr size_t INODE_INDIRECT_ADDRESS_NUM = 1;
constexpr size_t INODE_SIZE = 64;

// directory entry
/* Unit: bit
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|      ENTRY SIZE       |       FNAME SIZE      |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
|                 INODE NUMBER                  |
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
/                  <FILE NAME>                  /
/                   <PADDING>                   /
+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 */
typedef unsigned char dent_size_t;
typedef unsigned char dent_fname_size_t;
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
constexpr size_t INODES_BITMAP_SIZE = INODES_NUM_MAX / CHAR_BIT;

constexpr size_t BLOCKS_BITMAP_START = INODES_BITMAP_START + INODES_BITMAP_SIZE;
constexpr size_t BLOCKS_BITMAP_SIZE = BLOCK_NUM / CHAR_BIT;

constexpr size_t INODES_START = BLOCKS_BITMAP_START + BLOCKS_BITMAP_SIZE;
constexpr size_t INODES_SIZE = INODES_NUM_MAX * INODE_SIZE;

constexpr size_t BLOCKS_START = INODES_START + INODES_SIZE;
