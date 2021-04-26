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

/**
 * Represents a frame rate in Hz.
 */
struct FrameRate {
  constexpr FrameRate(std::uint32_t numerator_, uint32_t denominator_)
      : numerator{numerator_}, denominator{denominator_} {}

  constexpr FrameRate() : FrameRate{0, 1} {}

  std::uint32_t numerator;
  std::uint32_t denominator;
};

} // namespace camcoder
