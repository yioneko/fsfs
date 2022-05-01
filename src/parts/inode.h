#ifndef INODE_H
#define INODE_H

#include "../config.h"
#include "../disk.h"
#include <array>
#include <initializer_list>
#include <vector>

struct Inode {
  i_mode_t mode;
  i_uid_t uid;
  i_gid_t gid;
  i_fsize_t size;
  i_time_t atime;
  i_time_t mtime;
  std::array<blk_num_t, INODE_DIRECT_ADDRESS_NUM> direct_addresses;
  std::array<blk_num_t, INODE_INDIRECT_ADDRESS_NUM> indirect_addresses;

  std::vector<std::array<blk_num_t, INODE_INDIRECT_BLOCK_ADDRESS_NUM>>
      indirect_block_addresses; // The length of this is variant because the
                                // block may not exist

  Inode(i_mode_t mode, i_uid_t uid, i_gid_t gid);

  typedef std::vector<std::pair<blk_num_t, std::array<byte, BLOCK_SIZE>>>
      indirect_block_bytes_t;
  std::pair<std::array<byte, INODE_SIZE>, indirect_block_bytes_t> &&
  to_bytes() const;

  static Inode &&read_from_disk(const Disk &, const size_t offset);

  void expand_indirect_addresses(std::initializer_list<blk_num_t> blocks);

private:
  Inode(i_mode_t mode, i_uid_t uid, i_gid_t gid, i_fsize_t size, i_time_t atime,
        i_time_t mtime);
};

#endif /* INODE_H */
