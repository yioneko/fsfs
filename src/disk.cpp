#include "disk.h"
#include <fstream>
#include <stdexcept>

Disk::Disk() { this->data.fill(0); }

Disk Disk::load(const std::string &path) {
  Disk disk;

  std::ifstream file(path, std::ios::binary);
  if (!file.is_open()) {
    throw std::runtime_error("Could not open file: " + path);
  }
  file.read(reinterpret_cast<char *>(disk.data.data()), disk.data.size());
  file.close();
  return disk;
}

void Disk::save(const std::string &path) const {
  std::ofstream file(path, std::ios::binary | std::ios::trunc);
  if (!file.is_open()) {
    throw std::runtime_error("Could not open file: " + path);
  }
  file.write(reinterpret_cast<const char *>(this->data.data()),
             this->data.size());
  file.close();
}
