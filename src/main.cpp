#include <chrono>
#include <thread>

#include "BlockingCollection.h"

#include "pipeline.hpp"
#include "frame_source.hpp"

using namespace camcoder;

static code_machina::BlockingQueue<std::unique_ptr<RGBFrame>> frame_q{100};

static const FrameParameters frame_params{640, 480, PixelFormat::RGB};

static void frame_producer_thread() {
  using namespace std::chrono_literals;
  FileFrameSource f{"out.bin", frame_params};
  while (f) {
    // std::this_thread::sleep_for(32ms);
    auto pframe = RGBFrame::cast(f.get_frame());
    frame_q.add(std::move(pframe));
    std::cout << "Pushed" << std::endl;
  }
  frame_q.complete_adding();
  std::cout << "Producer done" << std::endl;
}

static void frame_consumer_thread(Pipeline &p) {
  // frame_q is completed when complete_adding() has been called and the queue
  // is empty
  int i = 0;
  while (!frame_q.is_completed()) {
    std::unique_ptr<RGBFrame> pframe;
    frame_q.take(pframe);
    std::cout << "Popped" << std::endl;
    using namespace std::chrono_literals;
    p.push_frame(std::move(pframe), i * 32ms);
    i++;
  }
  p.stop();
  std::cout << "Consumer done" << std::endl;
}

int main() {
  Gst::init();
  Pipeline p{frame_params};
  // std::thread pipeline_thread{std::ref(p)};
  // std::thread consumer_thread{frame_consumer_thread, std::ref(p)};
  // std::thread producer_thread{frame_producer_thread};
  // producer_thread.join();
  // consumer_thread.join();
  // pipeline_thread.join();
  p();
}
