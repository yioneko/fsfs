# FSFS

A simple in-memory file system with capability of being mounted on existing file system. Only for educational purpose.

## Usage

FSFS requires libfuse3, for Ubuntu/Debian users:

```sh
$ apt install libfuse3-3
```

CLI usage:

```
$ fsfs --file=<file> [FUSE_OPTIONS] <mountpoint>
```

`--file=<file>` is required for persistence of data.

## Compile

For Ubuntu/Debian users:

```sh
$ apt install libfuse3-dev
```

And follow the [official guide](https://xmake.io/#/guide/installation) to install xmake. Then run `xmake build` to compile the binary.
