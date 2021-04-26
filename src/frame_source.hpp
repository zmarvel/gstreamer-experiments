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
      : frame_params_{frame_params} {}

  template <typename TFrame> TFrame get_frame();

  explicit operator bool() const { return good() && !eof(); }

  std::unique_ptr<Frame> get_frame_ptr();

  bool connected() const { return connected_(); }

  bool connect() { return connect_(); }

  bool finished() const { return eof(); }

  constexpr FrameParameters frame_parameters() const { return frame_params_; }

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
  FrameParameters frame_params_;
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

  constexpr size_t frame_count() const;

  void operator()();

  std::unique_ptr<Frame> pop_frame();

  FrameParameters frame_parameters() const;

private:
  std::unique_ptr<FrameSource> frame_source_;
  size_t frame_count_;
  code_machina::BlockingQueue<std::unique_ptr<Frame>> frame_q_;
  std::thread thread_;
};

template <> RGBFrame FrameSource::get_frame<RGBFrame>();

} // namespace camcoder
