#pragma once
#include <cstdlib>

namespace camcoder {
enum class PixelFormat {
  INVALID = 0,
  RGB = 1,
};

[[maybe_unused]] static constexpr const char *
pixel_format_to_string(PixelFormat format) {
  switch (format) {
  case PixelFormat::INVALID:
    return {};
  case PixelFormat::RGB:
    return "RGB";
  default:
    return {};
  }
}

struct FrameParameters {
  size_t width;
  size_t height;
  PixelFormat pixel_format;
};

} // namespace camcoder
