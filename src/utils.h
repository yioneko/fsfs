#include "disk.h"
#include <algorithm>
#include <cstring>

template <typename T, typename Iter>
T read_n(Iter &iter, size_t n = sizeof(T)) {
  byte byte_buffer[n];
  std::copy_n(iter, n, byte_buffer);
  iter += n;

  T res;
  memcpy(&res, byte_buffer, n);
  return res;
}

template <typename T, typename Iter>
void write_n(Iter &iter, T val, size_t n = sizeof(T)) {
  byte byte_buffer[n];
  memcpy(byte_buffer, &val, n);

  // forward iterator after writing
  iter = std::move(byte_buffer, byte_buffer + n, iter);
}
