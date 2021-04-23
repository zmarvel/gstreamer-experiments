#pragma once

#include <cstdlib>
#include <cstdint>

#include "frame_parameters.hpp"

namespace camcoder {
static constexpr size_t pixel_size(PixelFormat t);

template <PixelFormat pixel_format_> struct Pixel {
  static constexpr PixelFormat format() { return pixel_format_; }
  static constexpr size_t size() { return pixel_size(format()); }
};

struct RGBPixel : public Pixel<PixelFormat::RGB> {
  std::uint8_t r;
  std::uint8_t g;
  std::uint8_t b;
};

static constexpr size_t pixel_size(PixelFormat format) {
  switch (format) {
  case PixelFormat::INVALID:
    return 0;
  case PixelFormat::RGB:
    return sizeof(RGBPixel);
  default:
    return 0;
  }
}

class Frame {
public:
  Frame() = delete;
  virtual ~Frame() = default;

  constexpr size_t width() const { return params_.width; }
  constexpr size_t height() const { return params_.height; }

  constexpr size_t size_pixels() const { return width() * height(); }
  constexpr size_t size_bytes() const {
    return size_pixels() * pixel_size(params_.pixel_format);
  }

  constexpr PixelFormat pixel_format() const { return params_.pixel_format; }

protected:
  Frame(const FrameParameters &params) : params_{params} {}

private:
  FrameParameters params_;
};

// No use specifying frame parameters at compile time since they'll be
// determined by a config
template <typename TPixel> struct FrameTmpl : public Frame {
  FrameTmpl(size_t width, size_t height)
      : Frame{{width, height, TPixel::format()}},
        data_{new TPixel[size_bytes()]} {}

  using pixel_type = TPixel;

  static constexpr size_t pixel_size() { return sizeof(pixel_type); }
  constexpr size_t size_bytes() const { return size_pixels() * pixel_size(); }

  TPixel *data() { return reinterpret_cast<TPixel *>(data_.get()); }

  TPixel &at(size_t x, size_t y) {
    size_t i = y * width() + x;
    if (i < size_pixels()) {
      return data_.get()[i];
    } else {
      throw std::out_of_range{"Frame access out of range"};
    }
  }

  const TPixel &at(size_t x, size_t y) const { return at(x, y); }

  static constexpr FrameTmpl<TPixel> &cast(Frame &f) {
    if (TPixel::format() == f.pixel_format()) {
      return static_cast<FrameTmpl<TPixel> &>(f);
    } else {
      throw std::bad_cast{"Frame format doesn't match"};
    }
  }

  static constexpr std::unique_ptr<FrameTmpl<TPixel>>
  cast(std::unique_ptr<Frame> f) {
    if (TPixel::format() == f->pixel_format()) {
      return std::unique_ptr<FrameTmpl<TPixel>>(
          dynamic_cast<FrameTmpl<TPixel> *>(f.release()));
    } else {
      throw std::bad_cast{};
    }
  }

private:
  std::unique_ptr<TPixel[]> data_;
};

using RGBFrame = FrameTmpl<RGBPixel>;
} // namespace camcoder
