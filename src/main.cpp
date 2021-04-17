#include "pipeline.hpp"
#include "frame_source.hpp"

using namespace camcoder;

int main() {
  Gst::init();
  Pipeline p{640, 480, "RGB"};
  p();
  // FileFrameSource f{"out.bin", 640, 480, PixelFormat::RGB};
  // while (f) {
  //   auto pframe = RGBFrame::cast(f.get_frame());
  // }
}
