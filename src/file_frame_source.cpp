#include <spdlog/spdlog.h>

#include "file_frame_source.hpp"

using namespace camcoder;

FileFrameSource::FileFrameSource(const std::string &path,
                                 const FrameParameters &frame_params)
    : FrameSource{frame_params}, path_{path}, ifs_{path}, loop_{false} {}

std::unique_ptr<FileFrameSource>
FileFrameSource::from_config(const FrameSourceConfig &config) {

  const auto path_it = config.options.find("path");
  if (path_it == config.options.end()) {
    spdlog::error("Option path is required for source {}", config.name);
    return nullptr;
  }

  auto frame_source = std::make_unique<FileFrameSource>(
      path_it->second.as_string(), config.frame_params);

  const auto loop_it = config.options.find("loop");
  if (loop_it != config.options.end()) {
    frame_source->enable_loop(loop_it->second.as_boolean());
  }

  spdlog::info("Creating FileFrameSource<path={}, loop={}>",
               frame_source->path(), loop_it->second.as_boolean());

  return frame_source;
}

bool FileFrameSource::seek(ptrdiff_t pos) {
  // if (ifs_.eof()) {
  // ifs_.clear(ifs_.rdstate() & (~std::ifstream::eofbit));
  // }
  ifs_.clear();
  ifs_.seekg(pos, std::ifstream::beg);
  return ifs_.good();
}

void FileFrameSource::enable_loop(bool enabled) { loop_ = enabled; }

size_t FileFrameSource::read(char *buf, size_t n) {
  if (ifs_.read(buf, n)) {
    return n;
  } else {
    return 0;
  }
}

bool FileFrameSource::eof() const { return !loop_ && ifs_.eof(); }

bool FileFrameSource::good() const { return ifs_.good(); }

bool FileFrameSource::bad() const { return ifs_.bad(); }

bool FileFrameSource::connected_() const { return !ifs_.eof(); }

bool FileFrameSource::connect_() {
  if (loop_ && ifs_.eof()) {
    spdlog::debug("Rewinding file");
    return seek(0);
  } else {
    return connected_();
  }
}
