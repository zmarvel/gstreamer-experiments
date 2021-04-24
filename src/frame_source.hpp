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

  template <typename TFrame> TFrame get_frame();
  explicit operator bool() const { return good() && !eof(); }

  bool end() const { return eof(); }

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

  void seek(ptrdiff_t pos) { ifs_.seekg(pos, std::ifstream::beg); }
  void clear() { ifs_.clear(); }

private:
  size_t read(char *buf, size_t n) override {
    if (ifs_.read(buf, n)) {
      return n;
    } else {
      return 0;
    }
  }

  bool eof() const override { return ifs_.eof(); }

  bool good() const override { return ifs_.good(); }
  bool bad() const { return ifs_.bad(); }

  std::ifstream ifs_;
};

template <> RGBFrame FrameSource::get_frame<RGBFrame>() {
  // This could also be a template. We wouldn't need frame_params_ if the
  // caller provided that information via type.
  auto frame = RGBFrame{frame_params_.width, frame_params_.height};
  const auto size = frame_size_bytes();
  if (read(reinterpret_cast<char *>(frame.data()), size) != size) {
    throw std::runtime_error{"Failed to read frame"};
  }
  return frame;
}

} // namespace camcoder
