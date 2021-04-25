#pragma once

#include <vector>
#include <string>
#include <map>
#include <istream>
#include <string_view>

#include "frame_parameters.hpp"

namespace camcoder {

/**
 * Type of a single frame source.
 */
enum class FrameSourceType {
  INVALID = 0,
  FILE,
  TCP_CLIENT,
  TCP_SERVER,
};

/**
 * Configuration for a single frame source.
 */
struct FrameSourceConfig {
  FrameSourceType type;
  /**
   * Frame parameters (width, height, pixel type).
   */
  FrameParameters frame_params;

  /**
   * Options specific to each type of frame source.
   */
  std::map<std::string, std::string> options;
};

/**
 * Configuration for a running instance of camcoder.
 */
class Config {
public:
  Config();
  Config(const std::string &path);
  Config(std::istream &&is, const std::string &path = "");

  /**
   * True if config was loaded from the given file, otherwise false (and the
   * defaults will be used).
   */
  constexpr bool loaded() const { return loaded_; }

  /**
   * This is where the HTTP server should look for the playlist file and the
   * MPEG-TS files.
   */
  std::string output_directory;
  static constexpr std::string_view DEFAULT_OUTPUT_DIRECTORY{"."};

  /**
   * This is the list of configs for the frame sources.
   */
  std::vector<FrameSourceConfig> frame_sources;

private:
  bool loaded_;
};

} // namespace camcoder
