#ifndef INODE_H
#define INODE_H

#include "../config.h"
#include "../disk.h"
#include <array>

struct Inode {
  i_mode_t mode;
  i_uid_t uid;
  i_gid_t gid;
  i_fsize_t size;
  i_time_t atime;
  i_time_t mtime;
  std::array<blk_num_t, INODE_DIRECT_ADDRESS_NUM> direct_addresses;
  std::array<blk_num_t, INODE_INDIRECT_ADDRESS_NUM> indirect_addresses;

  Inode() = delete;
  Inode(i_mode_t mode, i_uid_t uid, i_gid_t gid);

  std::array<byte, INODE_SIZE> &&to_bytes() const;

  static Inode &&read_from_disk(const Disk &, const size_t offset);
};

#endif /* INODE_H */
