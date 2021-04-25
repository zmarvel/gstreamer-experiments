#include <fstream>

#include <toml.hpp>
#include <spdlog/spdlog.h>

#include "config.hpp"

using namespace camcoder;

Config::Config() : output_directory{DEFAULT_OUTPUT_DIRECTORY}, loaded_{false} {}

Config::Config(const std::string &path) : Config{std::ifstream{path}, path} {}

Config::Config(std::istream &&is, const std::string &path) : Config{} {
  if (!is) {
    spdlog::warn("Failed to load config from {}; defaults will be used", path);
    return;
  }

  auto root = toml::parse(is);

  // project top-level settings
  if (root.contains("output_directory")) {
    output_directory = toml::find<std::string>(root, "output_directory");
  }

  loaded_ = true;
}
