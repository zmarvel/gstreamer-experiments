#include <chrono>
#include <thread>
#include <assert.h>

#include <sockpp/socket.h>
#include <spdlog/spdlog.h>
#include "BlockingCollection.h"
#include <cargs.h>

#include "pipeline.hpp"
#include "file_frame_source.hpp"
#include "tcp_server_frame_source.hpp"
#include "tcp_client_frame_source.hpp"
#include "config.hpp"

using namespace camcoder;

static code_machina::BlockingQueue<RGBFrame> frame_q{100};

static const FrameParameters frame_params{640, 480, PixelFormat::RGB};

#if 0
RGBFrame generate_frame(int i) {
  RGBFrame frame(frame_params.width, frame_params.height);
  for (size_t row = 0; row < frame.height(); row++) {
    const uint8_t red = (static_cast<float>(row) / frame.height()) * 255.9;
    const uint8_t blue =
        ((frame.height() - static_cast<float>(row)) / frame.height()) * 255.9;
    for (size_t col = 0; col < frame.width(); col++) {
      frame.at(col, row) = RGBPixel{red, static_cast<uint8_t>(i % 256), blue};
    }
  }
  return frame;
}
#endif

[[maybe_unused]] static std::ostream &operator<<(std::ostream &os,
                                                 const RGBPixel &pix) {
  os << static_cast<uint32_t>(pix.r) << " " << static_cast<uint32_t>(pix.g)
     << " " << static_cast<uint32_t>(pix.b);
  return os;
}

static cag_option options[] = {{
                                   .identifier = 'c',
                                   .access_letters = "c",
                                   .access_name = "config",
                                   .value_name = "CONFIG_PATH",
                                   .description = "Path to config file",
                               },
                               {
                                   .identifier = 'v',
                                   .access_letters = "v",
                                   .access_name = "verbose",
                                   .value_name = nullptr,
                                   .description = "Enable verbose output",
                               },
                               {
                                   .identifier = 'h',
                                   .access_letters = "h",
                                   .access_name = "help",
                                   .value_name = nullptr,
                                   .description = "Show the command usage",
                               }};

int main(int argc, char *argv[]) {
  const char *config_path = "camcoder.toml";

  cag_option_context option_ctx{};
  cag_option_prepare(&option_ctx, options, CAG_ARRAY_SIZE(options), argc, argv);

  while (cag_option_fetch(&option_ctx)) {
    switch (cag_option_get(&option_ctx)) {
    case 'c':
      config_path = cag_option_get_value(&option_ctx);
      break;
    case 'v':
      spdlog::set_level(spdlog::level::debug);
      break;
    case 'h':
      std::cout << "Usage: camcoder [OPTION] ..." << std::endl;
      cag_option_print(options, CAG_ARRAY_SIZE(options), stdout);
      return 0;
    }
  }

  spdlog::info("Using config at {}", config_path);
  Config config{config_path};

  // Only needed if we're writing socket code, but for now we'll assume it
  // doesn't hurt to initialize it.
  sockpp::socket_initializer{};

  Gst::init();

  Pipeline p{config};

  for (const auto &conf : config.frame_sources) {
    std::unique_ptr<FrameSource> pframe_source{nullptr};
    switch (conf.type) {
    case FrameSourceType::FILE:
      pframe_source = FileFrameSource::from_config(conf);
      break;
    case FrameSourceType::TCP_CLIENT:
      pframe_source = TCPClientFrameSource::from_config(conf);
      break;
    case FrameSourceType::TCP_SERVER:
      pframe_source = TCPServerFrameSource::from_config(conf);
      break;
    default:
      break;
    }
    if (pframe_source != nullptr) {
      auto pframe_thread =
          std::make_unique<FrameThread>(std::move(pframe_source));
      p.add_frame_source(std::move(pframe_thread));
    } else {
      spdlog::warn("Failed to construct frame source from config for {}",
                   conf.name);
    }
  }

  p();
}
