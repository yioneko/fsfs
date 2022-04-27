#ifndef DISK_H
#define DISK_H

#include "config.h"
#include <array>
#include <string>

using byte = unsigned char;

class Disk {
public:
  Disk();
  ~Disk();
  void save(const std::string &path);
  static Disk &&load(const std::string &path);

  auto begin() { return this->data.begin(); }
  auto end() { return this->data.end(); }
  auto cbegin() { return this->data.cbegin(); }
  auto cend() { return this->data.cend(); }

private:
  std::array<byte, DISK_SIZE> data;
};

#endif /* DISK_H */
