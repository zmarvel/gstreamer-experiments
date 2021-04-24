#include "file_frame_source.hpp"

using namespace camcoder;

FileFrameSource::FileFrameSource(const std::string &path,
                                 const FrameParameters &frame_params)
    : FrameSource{frame_params}, ifs_{path} {}

void FileFrameSource::seek(ptrdiff_t pos) {
  ifs_.seekg(pos, std::ifstream::beg);
}

void FileFrameSource::clear() { ifs_.clear(); }

size_t FileFrameSource::read(char *buf, size_t n) {
  if (ifs_.read(buf, n)) {
    return n;
  } else {
    return 0;
  }
}

bool FileFrameSource::eof() const { return ifs_.eof(); }

bool FileFrameSource::good() const { return ifs_.good(); }

bool FileFrameSource::bad() const { return ifs_.bad(); }

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
