#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION 31

#include <cstddef>
#include <errno.h>
#include <fuse3/fuse.h>
#include <fuse3/fuse_lowlevel.h>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

static struct options {
  char *file;
  int show_help;
} options;

static const struct fuse_opt option_spec[] = {
    {"--file=%s", offsetof(struct options, file), 0},
    {"-h", offsetof(struct options, show_help), 1},
    FUSE_OPT_END};

static void show_help(const char *progname) {
  std::cout << "Usage: " << progname << " [OPTIONS] <mountpoint>\n"
            << "    --file=<file>       file to save/load the disk\n";
  fuse_cmdline_help();
}

int main(int argc, char *argv[]) {
  struct fuse_operations operations;
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

  if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1) {
    return 1;
  }

  if (options.show_help) {
    show_help(argv[0]);
  } else {
    if (options.file == nullptr) {
      std::cerr << "Missing file" << std::endl;
      return 1;
    }
  }

  return fuse_main(args.argc, args.argv, &operations, NULL);
}
