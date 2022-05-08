#ifndef DISK_H
#define DISK_H

#include "config.h"
#include <array>
#include <string>

using byte = unsigned char;

class Disk {
public:
  Disk();
  void save(const std::string &path) const;
  static Disk load(const std::string &path);

  auto begin() { return this->data.begin(); }
  auto end() { return this->data.end(); }
  auto cbegin() const { return this->data.cbegin(); }
  auto cend() const { return this->data.cend(); }

private:
  std::array<byte, DISK_SIZE> data;
};

#endif /* DISK_H */
