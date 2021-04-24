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

  bool finished() const { return eof(); }

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

} // namespace camcoder
