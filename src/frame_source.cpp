#include "frame_source.hpp"

using namespace camcoder;

template <> RGBFrame FrameSource::get_frame<RGBFrame>() {
  auto frame = RGBFrame{frame_params_.width, frame_params_.height};
  const auto size = frame_size_bytes();
  if (read(reinterpret_cast<char *>(frame.data()), size) != size) {
    throw std::runtime_error{"Failed to read frame"};
  }
  return frame;
}

std::unique_ptr<Frame> FrameSource::get_frame_ptr() {
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
}
