#include "pipeline.hpp"

using namespace camcoder;

int main() {
  Gst::init();
  Pipeline p{};
  p();
}
