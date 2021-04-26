#pragma once

#include "frame_source.hpp"
#include "config.hpp"

namespace camcoder {

class FileFrameSource : public FrameSource {
public:
  FileFrameSource(const std::string &path, const FrameParameters &frame_params);

  static std::unique_ptr<FileFrameSource>
  from_config(const FrameSourceConfig &config);

  bool seek(ptrdiff_t pos);

  void enable_loop(bool enabled);

  const std::string& path() const { return path_;}

private:
  size_t read(char *buf, size_t n) override;

  bool eof() const override;

  bool good() const override;

  bool bad() const override;

  bool connected_() const override;

  bool connect_() override;

std::string path_;
  std::ifstream ifs_;
  bool loop_;
};

} // namespace camcoder
