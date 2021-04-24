#pragma once

#include "frame_source.hpp"

namespace camcoder {

class FileFrameSource : public FrameSource {
public:
  FileFrameSource(const std::string &path, const FrameParameters &frame_params);

  void seek(ptrdiff_t pos);
  void clear();

private:
  size_t read(char *buf, size_t n) override;

  bool eof() const override;

  bool good() const override;
  bool bad() const override;

  std::ifstream ifs_;
};

template <> RGBFrame FrameSource::get_frame<RGBFrame>();

} // namespace camcoder
