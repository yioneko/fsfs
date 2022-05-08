#include "inode.h"
#include "../utils.h"
#include <algorithm>
#include <cstring>
#include <ctime>
#include <stdexcept>

Inode::Inode(i_mode_t mode, i_uid_t uid, i_gid_t gid)
    : mode(mode), uid(uid), gid(gid), size(0), atime(time(nullptr)),
      mtime(time(nullptr)) {
  this->direct_addresses.fill(0);
  this->indirect_addresses.fill(0);
}

Inode::Inode(i_mode_t mode, i_uid_t uid, i_gid_t gid, i_fsize_t size,
             i_time_t atime, i_time_t mtime)
    : mode(mode), uid(uid), gid(gid), size(size), atime(atime), mtime(mtime) {
  this->direct_addresses.fill(0);
  this->indirect_addresses.fill(0);
}

Inode Inode::read_from_disk(const Disk &disk, const size_t offset) {
  auto iter = disk.cbegin() + offset;
  auto mode = read_n<i_mode_t>(iter);
  auto uid = read_n<i_uid_t>(iter);
  auto gid = read_n<i_gid_t>(iter);
  auto size = read_n<i_fsize_t>(iter);
  auto atime = read_n<i_time_t>(iter);
  auto mtime = read_n<i_time_t>(iter);

  Inode res(mode, uid, gid, size, atime, mtime);
  for (auto i = 0; i < INODE_DIRECT_ADDRESS_NUM; ++i) {
    res.direct_addresses[i] = read_n<blk_num_t>(iter);
  }
  for (auto i = 0; i < INODE_INDIRECT_ADDRESS_NUM; ++i) {
    auto indirect_blk_num = read_n<blk_num_t>(iter);
    res.indirect_addresses[i] = indirect_blk_num;
    if (indirect_blk_num != 0) {
      res.indirect_block_addresses.emplace_back();

      auto indirect_blk_iter =
          disk.cbegin() + get_data_block_address(indirect_blk_num);
      for (auto j = 0; j < INODE_INDIRECT_BLOCK_ADDRESS_NUM; ++j) {
        auto direct_blk_num = read_n<blk_num_t>(indirect_blk_iter);
        res.indirect_block_addresses[i][j] = direct_blk_num;
      }
    }
  }

  return res;
}

std::pair<std::array<byte, INODE_SIZE>, Inode::indirect_block_bytes_t>
Inode::to_bytes() const {
  std::array<byte, INODE_SIZE> bytes;
  auto iter = bytes.begin();

  write_n(iter, mode);
  write_n(iter, uid);
  write_n(iter, gid);
  write_n(iter, size);
  write_n(iter, atime);
  write_n(iter, mtime);

  for (auto i = 0; i < INODE_DIRECT_ADDRESS_NUM; ++i) {
    write_n(iter, direct_addresses[i]);
  }
  for (auto i = 0; i < INODE_INDIRECT_ADDRESS_NUM; ++i) {
    write_n(iter, indirect_addresses[i]);
  }

  indirect_block_bytes_t indirect_block_bytes;
  for (auto i = 0; i < INODE_INDIRECT_ADDRESS_NUM; ++i) {
    if (indirect_addresses[i] != 0) {
      indirect_block_bytes.emplace_back();
      auto &indirect_block_bytes_i = indirect_block_bytes[i];
      indirect_block_bytes_i.first = indirect_addresses[i];

      auto indirect_block_iter = indirect_block_bytes_i.second.begin();
      for (auto j = 0; j < INODE_INDIRECT_BLOCK_ADDRESS_NUM; ++j) {
        write_n(indirect_block_iter, indirect_block_addresses[i][j]);
      }
    }
  }

  return std::make_pair(bytes, indirect_block_bytes);
}

void Inode::expand_indirect_addresses(std::initializer_list<blk_num_t> blocks) {
  const auto old_size = this->indirect_block_addresses.size();
  if (blocks.size() + old_size > INODE_INDIRECT_BLOCK_ADDRESS_NUM) {
    throw std::runtime_error("No more indirect addresses could be expanded");
  }
  this->indirect_block_addresses.resize(old_size + blocks.size());
  auto blk_iter = blocks.begin();
  for (auto i = old_size; i < old_size + blocks.size(); ++i, ++blk_iter) {
    this->indirect_addresses[i] = *blk_iter;
    this->indirect_block_addresses[i].fill(0);
  }
}

std::vector<blk_num_t> Inode::get_refer_blk_nums() const {
  std::vector<blk_num_t> res;
  for (auto i = 0; i < INODE_DIRECT_ADDRESS_NUM; ++i) {
    if (direct_addresses[i] != 0) {
      res.push_back(direct_addresses[i]);
    }
  }
  for (auto i = 0; i < INODE_INDIRECT_ADDRESS_NUM; ++i) {
    if (indirect_addresses[i] != 0) {
      res.push_back(indirect_addresses[i]);
    }
  }
  for (auto &blk_addresses : indirect_block_addresses) {
    for (auto &blk_num : blk_addresses) {
      if (blk_num != 0) {
        res.push_back(blk_num);
      }
    }
  }
  return res;
}
