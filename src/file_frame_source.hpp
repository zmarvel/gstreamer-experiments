#pragma once

#include "frame_source.hpp"

namespace camcoder {

class FileFrameSource : public FrameSource {
public:
  FileFrameSource(const std::string &path, const FrameParameters &frame_params)
      : FrameSource{frame_params}, ifs_{path}, loop_{false} {}

  bool seek(ptrdiff_t pos) {
    if (ifs_.eof()) {
      ifs_.clear(ifs_.rdstate() & (~std::ifstream::eofbit));
    }
    ifs_.seekg(pos, std::ifstream::beg);
    return ifs_.good();
  }

  void enable_loop(bool enabled) { loop_ = enabled; }

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

  bool bad() const override { return ifs_.bad(); }

  bool connected_() const override { return !eof(); }

  bool connect_() override {
    if (loop_ && eof()) {
      return seek(0);
    } else {
      return connected_();
    }
  }

  std::ifstream ifs_;
  bool loop_;
};

} // namespace camcoder
