#include "frame_source.hpp"

using namespace camcoder;

template <> RGBFrame FrameSource::get_frame<RGBFrame>() {
  auto frame =
      RGBFrame{frame_params_.width, frame_params_.height, frame_count_++};
  if (frame.timestamp().count() == 0 && frame_rate().numerator == 0) {
    // Use the receive time as a timestamp
    frame.set_timestamp(std::chrono::system_clock::now().time_since_epoch());
  }
  const auto size = frame_size_bytes();
  if (read(reinterpret_cast<char *>(frame.data()), size) != size) {
    throw std::runtime_error{"Failed to read frame"};
  }
  return frame;
}

std::unique_ptr<Frame> FrameSource::get_frame_ptr() {
  try {
    switch (frame_parameters().pixel_format) {
    case PixelFormat::RGB: {
      auto pframe = std::make_unique<RGBFrame>();
      *pframe = std::move(get_frame<RGBFrame>());
      return pframe;
    } break;
    case PixelFormat::INVALID:
    default:
      return nullptr;
      break;
    }
  } catch (std::runtime_error &) {
    return nullptr;
  }
}

FrameThread::FrameThread(std::unique_ptr<FrameSource> frame_source,
                         size_t queue_size)
    : frame_source_{std::move(frame_source)}, frame_q_{queue_size},
      thread_{std::ref(*this)} {}

// This should let us do e.g.
//   FrameThread frame_thread{TCPServerFrameSource{...}}
// template <
//     typename TFrameSource,
//     typename = std::enable_if_t<std::is_base_of_v<FrameSource,
//     TFrameSource>>>
// FrameThread(TFrameSource &&frame_source,
//             size_t queue_size = DEFAULT_QUEUE_SIZE)
//     : FrameThread{std::make_unique<TFrameSource>(frame_source), queue_size}
//     {}

std::uint64_t FrameThread::frame_count() const {
  return frame_source_->frame_count();
}

void FrameThread::operator()() {
  spdlog::info("Frame source started");
  while (!frame_source_->finished()) {
    if (!frame_source_->connected()) {
      spdlog::debug("Reconnecting");
      if (!frame_source_->connect()) {
        // If we failed to reconnect, wait a little bit so we don't just spin
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1ms);
      }
    }
    auto pframe = frame_source_->get_frame_ptr();
    if (pframe != nullptr) {
      spdlog::debug("Add frame {} at {}", frame_count(),
                    reinterpret_cast<void *>(pframe.get()));
      frame_q_.add(std::move(pframe));
    }
  }
  frame_q_.complete_adding();
  // TODO: thread name
  spdlog::info("Frame source done");
}

std::unique_ptr<Frame> FrameThread::pop_frame() {
  std::unique_ptr<Frame> pframe;
  frame_q_.take(pframe);
  spdlog::debug("Take frame at {}", reinterpret_cast<void *>(pframe.get()));
  return pframe;
}

FrameParameters FrameThread::frame_parameters() const {
  return frame_source_->frame_parameters();
}

FrameRate FrameThread::frame_rate() const {
  return frame_source_->frame_rate();
}
