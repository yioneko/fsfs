#include "disk.h"
#include <algorithm>
#include <cstring>
#include <sstream>
#include <vector>

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

inline std::vector<std::string> split_path(const std::string &path) {
  std::vector<std::string> path_parts;
  std::stringstream path_stream(path);
  std::string path_part;
  while (std::getline(path_stream, path_part, '/')) {
    if (!path_part.empty()) {
      path_parts.push_back(path_part);
    }
  }
  return path_parts;
}

inline std::string parent_path(const std::string &path) {
  if (path == "/") {
    return path;
  }
  const auto last_slash = path.rfind('/');
  if (last_slash == std::string::npos) {
    return "/";
  }
  return path.substr(0, last_slash);
}

inline std::string basename(const std::string &path) {
  const auto last_slash = path.rfind('/');
  if (last_slash == std::string::npos) {
    return path;
  }
  return path.substr(last_slash + 1);
}
