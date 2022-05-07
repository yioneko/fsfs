#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION 31

#include "config.h"
#include "fd_iter.h"
#include "fs.h"
#include "utils.h"
#include <cstddef>
#include <errno.h>
#include <fuse3/fuse.h>
#include <fuse3/fuse_lowlevel.h>
#include <fuse3/fuse_opt.h>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

static struct options {
  char *file;
  int show_help;
} options;

static FS *fs = nullptr;

static const struct fuse_opt option_spec[] = {
    {"--file=%s", offsetof(struct options, file), 0},
    {"-h", offsetof(struct options, show_help), 1},
    FUSE_OPT_END};

static void show_help(const char *progname) {
  std::cout << "Usage: " << progname << " [OPTIONS] <mountpoint>\n"
            << "    --file=<file>       file to save/load the disk\n";
  fuse_cmdline_help();
}

static void fsfs_destroy(void *) {
  fs->dump(options.file);
  delete fs;
  fs = nullptr;
}

static int fsfs_getattr(const char *path, struct stat *stat,
                        struct fuse_file_info *) {
  try {
    const auto dirent = fs->get_dirent(path);
    const auto inode = fs->get_inode(dirent.inode_num);
    stat->st_mode = inode.mode;
    stat->st_uid = inode.uid;
    stat->st_gid = inode.gid;
    stat->st_size = inode.size;
    stat->st_atime = inode.atime;
    stat->st_mtime = inode.mtime;
    return 0;
  } catch (const std::exception &e) {
    return -ENOENT;
  }
}

static int fsfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                        off_t, struct fuse_file_info *,
                        enum fuse_readdir_flags) {
  try {
    const auto dirent = fs->get_dirent(path);
    const auto inode = fs->get_inode(dirent.inode_num);
    if (!S_ISDIR(inode.mode)) {
      return -ENOTDIR;
    }

    const auto dir = fs->get_dir_data(dirent.inode_num);
    for (const auto &entry : dir.dirents) {
      // TOOD: fill the stat buf
      filler(buf, entry.fname.c_str(), nullptr, 0,
             static_cast<fuse_fill_dir_flags>(0));
    }
    return 0;
  } catch (const std::exception &e) {
    return -ENOENT;
  }
}

static int fsfs_open(const char *path, struct fuse_file_info *fi) {
  try {
    const auto dirent = fs->get_dirent(path);
    const auto inode = fs->get_inode(dirent.inode_num);
    if (!S_ISREG(inode.mode)) {
      return -EISDIR;
    }
    return 0;
  } catch (const std::exception &e) {
    return -ENOENT;
  }
}

static int fsfs_create(const char *path, mode_t mode,
                       struct fuse_file_info *fi) {
  auto ctx = fuse_get_context();
  const auto dir_path = parent_path(path);
  try {
    const auto dir_inum =
        dir_path == "/" ? ROOT_INODE_NUM : fs->get_dirent(dir_path).inode_num;
    auto dir = fs->get_dir_data(dir_inum);
    auto dir_inode = fs->get_inode(dir_inum);

    const auto fname = basename(path);
    const auto new_inum = fs->alloc_inode();
    auto new_inode = Inode(mode, ctx->uid, ctx->gid);
    // TODO: allocate data block?
    fs->write_inode(new_inode, new_inum);

    dir.add_entry(fname, new_inum);
    // Directory is modified, so we need to write it back
    const auto dir_bytes = dir.to_bytes();
    fs->write_data(dir_bytes.begin(), dir_bytes.end(), dir_inode);
    fs->write_inode(dir_inode, dir_inum);

    return 0;
  } catch (const std::exception &e) {
    return -ENOENT;
  }
}

static int fsfs_read(const char *path, char *buf, size_t size, off_t offset,
                     struct fuse_file_info *) {
  try {
    const auto dirent = fs->get_dirent(path);
    const auto inode = fs->get_inode(dirent.inode_num);
    if (!S_ISREG(inode.mode)) {
      return -EISDIR;
    }

    auto fd_iter = fs->file_data_cbegin(inode);
    auto fd_iter_end = fs->file_data_cend(inode);
    auto read_size = 0;
    for (; fd_iter != fd_iter_end && read_size < size; ++fd_iter, ++read_size) {
      buf[read_size] = *fd_iter;
    }
    return read_size;

  } catch (const std::exception &e) {
    return -ENOENT;
  }
}

static int fsfs_write(const char *path, const char *buf, size_t size,
                      off_t offset, struct fuse_file_info *) {
  try {
    auto dirent = fs->get_dirent(path);
    auto inode = fs->get_inode(dirent.inode_num);
    if (!S_ISREG(inode.mode)) {
      return -EISDIR;
    }

    // TODO: handle insufficient space error
    const auto write_bytes = fs->write_data(buf, buf + size, inode, offset);
    fs->write_inode(inode, dirent.inode_num);
    return write_bytes;
  } catch (const std::exception &e) {
    return -ENOENT;
  }
}

static int fsfs_unlink(const char *path) {
  const auto dir_path = parent_path(path);
  try {
    const auto dir_inum =
        dir_path == "/" ? ROOT_INODE_NUM : fs->get_dirent(dir_path).inode_num;
    auto dir = fs->get_dir_data(dir_inum);
    auto dir_inode = fs->get_inode(dir_inum);

    const auto fname = basename(path);
    const auto dirent = dir.remove_entry(fname);
    const auto inode = fs->get_inode(dirent.inode_num);
    fs->free_inode_and_blocks(dirent.inode_num);

    // Directory is modified, so we need to write it back
    const auto dir_bytes = dir.to_bytes();
    fs->write_data(dir_bytes.begin(), dir_bytes.end(), dir_inode);
    fs->write_inode(dir_inode, dir_inum);

    return 0;
  } catch (const std::exception &e) {
    return -ENOENT;
  }
}

static int fsfs_mkdir(const char *path, mode_t mode) {
  auto ctx = fuse_get_context();
  const auto dir_path = parent_path(path);
  try {
    const auto parent_dir_inum =
        dir_path == "/" ? ROOT_INODE_NUM : fs->get_dirent(dir_path).inode_num;
    auto parent_dir = fs->get_dir_data(parent_dir_inum);
    auto parent_inode = fs->get_inode(parent_dir_inum);

    auto new_inode = Inode(mode | S_IFDIR, ctx->uid, ctx->gid);
    const auto new_inum = fs->alloc_inode();
    const auto new_dir = Dir(new_inum, parent_dir_inum);
    new_inode.size = new_dir.size();

    const auto new_dir_bytes = new_dir.to_bytes();
    fs->write_data(new_dir_bytes.begin(), new_dir_bytes.end(), new_inode);
    fs->write_inode(new_inode, new_inum);

    parent_dir.add_entry(basename(path), new_inum);
    const auto dir_bytes = parent_dir.to_bytes();
    fs->write_data(dir_bytes.begin(), dir_bytes.end(), parent_inode);
    fs->write_inode(parent_inode, parent_dir_inum);

    return 0;
  } catch (const std::exception &e) {
    return -ENOENT;
  }
}

static int fsfs_rmdir(const char *path) {
  // WARN: idk if this is correct...
  return fsfs_unlink(path);
}

static int fsfs_statfs(const char *, struct statvfs *stbuf) {
  stbuf->f_bsize = BLOCK_SIZE;
  stbuf->f_blocks = BLOCK_NUM_MAX;
  stbuf->f_bfree = BLOCK_NUM_MAX - fs->sb.used_blocks;
  stbuf->f_bavail = BLOCK_NUM_MAX - fs->sb.used_blocks;
  stbuf->f_files = INODES_NUM_MAX;
  stbuf->f_ffree = INODES_NUM_MAX - fs->sb.used_inodes;

  return 0;
}

int main(int argc, char *argv[]) {
  struct fuse_operations operations = {
      .getattr = fsfs_getattr,
      .mkdir = fsfs_mkdir,
      .unlink = fsfs_unlink,
      .rmdir = fsfs_rmdir,
      .open = fsfs_open,
      .read = fsfs_read,
      .write = fsfs_write,
      .statfs = fsfs_statfs,
      .readdir = fsfs_readdir,
  };
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

  if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1) {
    return 1;
  }
  fuse_opt_add_arg(&args, "-o default_permissions");

  if (options.show_help) {
    show_help(argv[0]);
  } else {
    if (options.file == nullptr) {
      std::cerr << "Missing file" << std::endl;
      return 1;
    }

    if (access(options.file, F_OK) != -1) {
      fs = new FS(options.file);
    } else {
      // If the file doesn't exist, initialize an empty valid filesystem
      // using the uid and gid of the calling process
      fs = new FS(getuid(), getgid());
    }
  }

  return fuse_main(args.argc, args.argv, &operations, NULL);
}
