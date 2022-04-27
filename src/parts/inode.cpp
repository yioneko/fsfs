#include "inode.h"
#include <ctime>

Inode::Inode(i_mode_t mode, i_uid_t uid, i_gid_t gid)
    : mode(mode), uid(uid), gid(gid), size(0), atime(time(nullptr)),
      mtime(time(nullptr)) {
  this->direct_addresses.fill(0);
  this->indirect_addresses.fill(0);
}
