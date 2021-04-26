#pragma once

#include <cstdint>
#include <memory>
#include <fstream>
#include <stdexcept>
#include <thread>

#include <BlockingCollection.h>
// TODO: move implementation out of header
#include <spdlog/spdlog.h>

#include "frame_parameters.hpp"
#include "frame.hpp"

namespace camcoder {

class FrameSource {
public:
  FrameSource(const FrameParameters &frame_params)
      : FrameSource{frame_params, FrameRate{0, 1}} {}

  FrameSource(const FrameParameters &frame_params, const FrameRate &frame_rate)
      : frame_count_{0}, frame_params_{frame_params}, frame_rate_{frame_rate} {}

  template <typename TFrame> TFrame get_frame();

  explicit operator bool() const { return good() && !eof(); }

  std::unique_ptr<Frame> get_frame_ptr();

  bool connected() const { return connected_(); }

  bool connect() { return connect_(); }

  bool finished() const { return eof(); }

  constexpr std::uint64_t frame_count() const { return frame_count_; }
  constexpr FrameParameters frame_parameters() const { return frame_params_; }
  constexpr FrameRate frame_rate() const { return frame_rate_; }

protected:
  virtual size_t read(char *buf, size_t n) = 0;
  virtual bool eof() const = 0;
  virtual bool good() const = 0;
  virtual bool bad() const = 0;
  virtual bool connected_() const = 0;
  virtual bool connect_() = 0;

  constexpr size_t frame_size_bytes() const {
    return frame_params_.width * frame_params_.height *
           pixel_size(frame_params_.pixel_format);
  }

private:
  std::uint64_t frame_count_;
  FrameParameters frame_params_;
  FrameRate frame_rate_;
};

// TODO: I think FrameThread could implement the FrameSource interface, too, and
// just pop something off the queue when get_frame is called. Not strictly
// necessary, but it could help keep the number of threads in the application
// down. Some thread sources (like FileFrameSource) don't need a thread to
// read--they can do it in the need-data callback.
class FrameThread {
public:
  static constexpr size_t DEFAULT_QUEUE_SIZE = 128;

  FrameThread(std::unique_ptr<FrameSource> frame_source,
              size_t queue_size = DEFAULT_QUEUE_SIZE);

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

  size_t frame_count() const;

  void operator()();

  std::unique_ptr<Frame> pop_frame();

  FrameParameters frame_parameters() const;
  FrameRate frame_rate() const;

private:
  std::unique_ptr<FrameSource> frame_source_;
  code_machina::BlockingQueue<std::unique_ptr<Frame>> frame_q_;
  std::thread thread_;
};

template <> RGBFrame FrameSource::get_frame<RGBFrame>();

} // namespace camcoder
