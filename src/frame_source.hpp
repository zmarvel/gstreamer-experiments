#pragma once

#include <cstdint>
#include <memory>
#include <fstream>
#include <stdexcept>

#include "frame_parameters.hpp"
#include "frame.hpp"

namespace camcoder {
class FrameSource {
public:
  FrameSource(const FrameParameters &frame_params)
      : frame_params_{frame_params} {}

  std::unique_ptr<Frame> get_frame() {
    // std::unique_ptr<Frame> frame;
    switch (frame_params_.pixel_format) {
    case PixelFormat::INVALID:
      return nullptr;
    case PixelFormat::RGB: {
      auto frame =
          std::make_unique<RGBFrame>(frame_params_.width, frame_params_.height);
      read(reinterpret_cast<char *>(frame->data()), frame_size_bytes());
      return frame;
    }
    default:
      return nullptr;
    }
  }

  explicit operator bool() const { return good() && !eof(); }

protected:
  virtual size_t read(char *buf, size_t n) = 0;
  virtual bool eof() const = 0;
  virtual bool good() const = 0;
  virtual bool bad() const = 0;

  constexpr size_t frame_size_bytes() const {
    return frame_params_.width * frame_params_.height *
           pixel_size(frame_params_.pixel_format);
  }

private:
  FrameParameters frame_params_;
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
  FileFrameSource(const std::string &path, const FrameParameters &frame_params)
      : FrameSource{frame_params}, ifs_{path} {}

private:
  size_t read(char *buf, size_t n) override {
    ifs_.get(buf, n);
    // TODO can we return the actual size read
    return n;
  }

  bool eof() const override { return ifs_.eof(); }

  bool good() const override { return ifs_.good(); }
  bool bad() const override { return ifs_.bad(); }

  std::ifstream ifs_;
};
} // namespace camcoder
