#include <chrono>
#include <thread>
#include <assert.h>

#include <sockpp/socket.h>
#include "BlockingCollection.h"

#include "pipeline.hpp"
#include "tcp_client_frame_source.hpp"

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

std::ostream &operator<<(std::ostream &os, const RGBPixel &pix) {
  os << static_cast<uint32_t>(pix.r) << " " << static_cast<uint32_t>(pix.g)
     << " " << static_cast<uint32_t>(pix.b);
  return os;
}

static void frame_producer_thread() {
  using namespace std::chrono_literals;
  TCPClientFrameSource frame_source{"127.0.0.1", 9000, frame_params};
  int i = 0;
  while (true) {
    if (!frame_source.connected()) {
      frame_source.connect();
    }
    i++;
    auto frame = frame_source.get_frame<RGBFrame>();
    frame_q.add(std::move(frame));
  }
  frame_q.complete_adding();
  std::cout << "Producer done" << std::endl;
}

static void frame_consumer_thread(Pipeline &p) {
  // frame_q is completed when complete_adding() has been called and the queue
  // is empty
  int i = 0;
  while (!frame_q.is_completed()) {
    RGBFrame frame;
    frame_q.take(frame);
    // std::cout << "Popped " << frame.size_bytes() << " B" << std::endl;
    assert(frame.size_bytes() == 3 * 640 * 480);
    using namespace std::chrono_literals;
    p.push_frame(frame, i * 32ms);
    i++;
  }
  p.stop();
  std::cout << "Consumer done" << std::endl;
}

int main() {
  // Only needed if we're writing socket code, but for now we'll assume it
  // doesn't hurt to initialize it.
  sockpp::socket_initializer{};

  Gst::init();
  Pipeline p{frame_params};
  std::thread pipeline_thread{std::ref(p)};
  std::thread consumer_thread{frame_consumer_thread, std::ref(p)};
  std::thread producer_thread{frame_producer_thread};
  producer_thread.join();
  consumer_thread.join();
  pipeline_thread.join();
}
