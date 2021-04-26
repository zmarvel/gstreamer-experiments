#include <fstream>
#include <unordered_map>

#include <toml.hpp>
#include <spdlog/spdlog.h>

#include "config.hpp"

using namespace camcoder;

Config::Config() : output_directory{DEFAULT_OUTPUT_DIRECTORY}, loaded_{false} {}

Config::Config(const std::string &path) : Config{std::ifstream{path}, path} {}

static const std::unordered_map<std::string, FrameSourceType>
    frame_source_type_from_string{
        {"file", FrameSourceType::FILE},
        {"tcp_client", FrameSourceType::TCP_CLIENT},
        {"tcp_server", FrameSourceType::TCP_SERVER},
    };

static const std::unordered_map<std::string, PixelFormat>
    pixel_format_from_string{
        {"RGB", PixelFormat::RGB},
    };

Config::Config(std::istream &&is, const std::string &path) : Config{} {
  if (!is) {
    spdlog::warn("Failed to load config from {}; defaults will be used", path);
    return;
  }

  root_ = toml::parse(is);

  // project top-level settings
  if (root_.contains("output_directory")) {
    output_directory = toml::find<std::string>(root_, "output_directory");
  }

  if (root_.contains("sources")) {
    for (const auto &[source_name, source_node] :
         toml::find(root_, "sources").as_table()) {
      // FrameSourceConfig{name, type, frame_params, options}
      // auto source_node = toml::find(root, source_name.as_string());

      if (!source_node.contains("type")) {
        spdlog::error("Source node {} missing required field type",
                      source_name);
        continue;
      }
      auto type_name = toml::find<std::string>(source_node, "type");
      const auto type = frame_source_type_from_string.find(type_name);
      if (type == frame_source_type_from_string.end()) {
        spdlog::error("Source node {} has invalid type {}", source_name,
                      type_name);
        continue;
      }

      // Read frame parameters
      FrameParameters frame_params{};
      if (!source_node.contains("frame_size")) {
        spdlog::error("Source node {} missing required parameter frame_size",
                      source_name);
        continue;
      } else if (!source_node.contains("pixel_format")) {
        spdlog::error("Source node {} missing required parameter pixel_format");
        continue;
      }
      const auto frame_size =
          toml::find<std::vector<int>>(source_node, "frame_size");
      if (frame_size.size() != 2) {
        spdlog::error(
            "Source node {} frame_size should have exactly 2 elements",
            source_name);
        continue;
      } else if (std::find_if(frame_size.begin(), frame_size.end(),
                              [](const auto &dim) { return dim <= 0; }) !=
                 frame_size.end()) {
        spdlog::error("Source node {} has invalid frame size", source_name);
        continue;
      }
      frame_params.width = frame_size[0];
      frame_params.height = frame_size[1];

      const auto pixel_format_name =
          toml::find<std::string>(source_node, "pixel_format");
      const auto pixel_format =
          pixel_format_from_string.find(pixel_format_name);
      if (pixel_format == pixel_format_from_string.end()) {
        spdlog::error("Source node {} has invalid pixel_format {}", source_name,
                      pixel_format_name);
        continue;
      }
      frame_params.pixel_format = pixel_format->second;

      // TODO: frame_rate

      frame_sources.push_back(FrameSourceConfig{
          .name = source_name,
          .type = type->second,
          .frame_params = frame_params,
          .options = source_node.as_table(),
      });
    }
  }

  loaded_ = true;
}
