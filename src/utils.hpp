#pragma once

#include <string>
#include <sstream>

namespace camcoder {
namespace utils {
/**
 * Append child to root. We assume that child is relative to root and does not
 * begin with a slash. We assume that / is the separator rather than \.
 */
static std::string path_join(const std::string &root,
                             const std::string &child) {
  std::stringstream ss;
  ss << root;
  if (root.back() != '/') {
    ss << '/';
  }
  ss << child;
  return ss.str();
}
} // namespace utils
} // namespace camcoder
