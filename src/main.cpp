#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION 31

#include <errno.h>
#include <fuse3/fuse.h>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

static struct Options {
  const char *filename;
  const char *contents;
} options;

static void *pt_init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
  cfg->use_ino = 1;
  cfg->entry_timeout = 0;
  cfg->attr_timeout = 0;
  cfg->negative_timeout = 0;

  return NULL;
}

static int pt_getattr(const char *path, struct stat *stbuf,
                      struct fuse_file_info *fi) {
  auto res = lstat(path, stbuf);
  if (res == -1)
    return -errno;
  return 0;
}

static int pt_access(const char *path, int mask) {
  auto res = access(path, mask);
  if (res == -1)
    return -errno;
  return 0;
}

int main(int argc, char **argv) {
  struct fuse_operations pt_oper = {
      .getattr = pt_getattr,
      .init = pt_init,
  };

  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

  options.filename = argv[1];
  options.contents = argv[2];

  return fuse_main(argc - 2, argv + 2, &pt_oper, NULL);
}
