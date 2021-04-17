#pragma once

#include <cstdint>
#include <memory>
#include <fstream>
#include <stdexcept>

// TODO namespace

enum class PixelFormat {
  INVALID = 0,
  RGB = 1,
};

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

static constexpr size_t pixel_size(PixelFormat t) {
  switch (t) {
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

  constexpr size_t width() const { return width_; }
  constexpr size_t height() const { return height_; }

  constexpr size_t size_pixels() const { return width() * height(); }
  constexpr size_t size_bytes() const {
    return size_pixels() * pixel_size(pixel_format_);
  }

  template <typename TPixel> const TPixel &at(size_t x, size_t y) const {}

  constexpr PixelFormat pixel_format() const { return pixel_format_; }

protected:
  Frame(size_t width, size_t height, PixelFormat pixel_format)
      : width_{width}, height_{height}, pixel_format_{pixel_format} {}

private:
  size_t width_;
  size_t height_;
  PixelFormat pixel_format_;
};

// No use specifying frame parameters at compile time since they'll be
// determined by a config
template <typename TPixel> struct FrameTmpl : public Frame {
  FrameTmpl(size_t width, size_t height)
      : Frame{width, height, TPixel::format()}, data_{
                                                    new TPixel[size_bytes()]} {}

  using pixel_type = TPixel;

  constexpr size_t pixel_size() { return sizeof(pixel_type); }

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

struct RGBFrame : public FrameTmpl<RGBPixel> {
  RGBFrame(size_t width, size_t height) : FrameTmpl{width, height} {}
};

class FrameSource {
public:
  FrameSource(size_t width, size_t height, PixelFormat pixel_format)
      : frame_width_{width}, frame_height_{height}, pixel_format_{
                                                        pixel_format} {}

  std::unique_ptr<Frame> get_frame() {
    // std::unique_ptr<Frame> frame;
    switch (pixel_format_) {
    case PixelFormat::INVALID:
      return nullptr;
    case PixelFormat::RGB: {
      auto frame = std::make_unique<RGBFrame>(frame_width_, frame_height_);
      read(reinterpret_cast<char *>(frame->data()), frame_size_bytes());
      return frame;
    }
    default:
      return nullptr;
    }
  }

  explicit operator bool() const { return !eof(); }

protected:
  virtual size_t read(char *buf, size_t n) = 0;
  virtual bool eof() const = 0;

  constexpr size_t frame_size_bytes() const {
    return frame_width_ * frame_height_ * pixel_size(pixel_format_);
  }

private:
  size_t frame_width_;
  size_t frame_height_;
  PixelFormat pixel_format_;
};

// TODO
#if 0
class TcpClientFrameSource : public FrameSource {
private:
  std::unique_ptr<char[]> read(size_t n) override {}
};
#endif

class FileFrameSource : public FrameSource {
public:
  FileFrameSource(const std::string &path, size_t width, size_t height,
                  PixelFormat pixel_format)
      : FrameSource{width, height, pixel_format}, ifs_{path} {}

private:
  size_t read(char *buf, size_t n) override {
    ifs_.get(buf, n);
    // TODO can we return the actual size read
    return n;
  }

  bool eof() const override { return ifs_.eof(); }

  std::ifstream ifs_;
};
