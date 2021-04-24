#pragma once

#include <cstdlib>
#include <cstdint>
#include <memory>

#include "frame_parameters.hpp"

namespace camcoder {
static constexpr size_t pixel_size(PixelFormat t);

template <PixelFormat pixel_format_> struct Pixel {
  static constexpr PixelFormat format() { return pixel_format_; }
  static constexpr size_t size() { return pixel_size(format()); }
};

#pragma pack(1)
struct RGBPixel : public Pixel<PixelFormat::RGB> {
  RGBPixel() = default;
  RGBPixel(uint8_t r_, uint8_t g_, uint8_t b_) : r{r_}, g{g_}, b{b_} {}

  std::uint8_t r;
  std::uint8_t g;
  std::uint8_t b;
};
#pragma pack()
static_assert(sizeof(RGBPixel) == 3);

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
  virtual ~Frame() = default;

  constexpr size_t width() const { return params_.width; }
  constexpr size_t height() const { return params_.height; }
  constexpr FrameParameters &params() { return params_; }
  constexpr const FrameParameters &params() const { return params_; }

  constexpr size_t size_pixels() const { return width() * height(); }
  constexpr size_t size_bytes() const {
    return size_pixels() * pixel_size(params_.pixel_format);
  }

  constexpr PixelFormat pixel_format() const { return params_.pixel_format; }

protected:
  Frame() : params_{} {}
  Frame(const FrameParameters &params) : params_{params} {}
  Frame(const Frame &other) = default;
  Frame &operator=(const Frame &other) = default;

private:
  FrameParameters params_;
};

// No use specifying frame parameters at compile time since they'll be
// determined by a config
template <typename TPixel> struct FrameTmpl : public Frame {
  FrameTmpl(size_t width, size_t height)
      : Frame{{width, height, TPixel::format()}},
        data_{new TPixel[size_bytes()]} {}

  FrameTmpl() : Frame{}, data_{nullptr} {}

  // No copy
  FrameTmpl(const FrameTmpl<TPixel> &other) = delete;
  FrameTmpl &operator=(const FrameTmpl<TPixel> &other) = delete;

  // Only move
  // Move constructor
  FrameTmpl(FrameTmpl<TPixel> &&other)
      : Frame{other.params()}, data_{std::move(other.data_)} {}

  // Move assignment operator
  FrameTmpl &operator=(FrameTmpl<TPixel> &&other) {
    Frame::operator=(std::move(other));
    std::swap(data_, other.data_);
    return *this;
  }

  using pixel_type = TPixel;

  static constexpr size_t pixel_size() { return sizeof(pixel_type); }
  constexpr size_t size_bytes() const { return size_pixels() * pixel_size(); }

  TPixel *data() { return reinterpret_cast<TPixel *>(data_.get()); }

  const TPixel *data() const {
    return reinterpret_cast<const TPixel *>(data_.get());
  }

  TPixel &at(size_t x, size_t y) {
    size_t i = y * width() + x;
    if (i < size_pixels()) {
      return data_.get()[i];
    } else {
      throw std::out_of_range{"Frame access out of range"};
    }
  }

  const TPixel &at(size_t x, size_t y) const {
    size_t i = y * width() + x;
    if (i < size_pixels()) {
      return data_.get()[i];
    } else {
      throw std::out_of_range{"Frame access out of range"};
    }
  }

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

static_assert(RGBFrame::pixel_size() == 3);

} // namespace camcoder
