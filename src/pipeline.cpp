
#include <iostream>

#include "pipeline.hpp"
#include "utils.hpp"

using namespace camcoder;

Pipeline::Pipeline(const Config &config, const FrameParameters &frame_params)
    : appsrc_{Gst::ElementFactory::create_element("appsrc")},
      convert_{Gst::ElementFactory::create_element("videoconvert")},
      encoder_{Gst::ElementFactory::create_element("x264enc")},
      // encoder_{Gst::ElementFactory::create_element("vaapih264enc")},
      // encoder_{Gst::ElementFactory::create_element("nvh264enc")},
      tsmux_{Gst::ElementFactory::create_element("mpegtsmux")},
      hls_sink_{Gst::ElementFactory::create_element("hlssink")},
      pipeline_{Gst::Pipeline::create()},
      terminate_{false}, playing_{false}, ready_{false} {

  // TODO check for null elements

  Gst::VideoInfo video_info;
  video_info.init();
  video_info.set_format(Gst::VideoFormat::VIDEO_FORMAT_RGB, frame_params.width,
                        frame_params.height);
  video_info.set_fps_n(30);
  video_info.set_fps_d(1);
  auto video_caps = video_info.to_caps();

  appsrc_->set_property("caps", video_caps);
  // appsrc_->set_property("format", GST_FORMAT_TIME);
  appsrc_->set_property("block", true);

  hls_sink_->set_property(
      "location", utils::path_join(config.output_directory, "segment%05d.ts"));
  hls_sink_->set_property(
      "playlist-location",
      utils::path_join(config.output_directory, "playlist.m3u8"));

  // TODO: use signals?

  pipeline_->add(appsrc_)->add(convert_)->add(encoder_)->add(tsmux_)->add(
      hls_sink_);
  appsrc_->link(convert_)->link(encoder_)->link(tsmux_)->link(hls_sink_);

  // pipeline_->add(appsrc_)->add(convert_)->add(hls_sink_);
  // appsrc_->link(convert_)->link(hls_sink_);
}

void Pipeline::operator()() {
  if (pipeline_->set_state(Gst::State::STATE_PLAYING) ==
      Gst::StateChangeReturn::STATE_CHANGE_FAILURE) {
    throw std::runtime_error{"Failed to change to playing state"};
  }
  std::cout << "Pipeline start" << std::endl;

  // Gst::Task
  // Gst::Buffer;
  // Glib::SignalProxy;

  // TODO: we can do this with a signal handler, I think--is that better?
  auto bus = pipeline_->get_bus();
  while (!terminate_) {
    auto msg = bus->pop(100 * Gst::MILLI_SECOND,
                        Gst::MessageType::MESSAGE_STATE_CHANGED |
                            Gst::MessageType::MESSAGE_ERROR |
                            Gst::MessageType::MESSAGE_EOS |
                            Gst::MessageType::MESSAGE_DURATION_CHANGED);

    if (msg) {
      handle_message(msg);
    } else {
      // The timeout expired
    }
  }
  pipeline_->set_state(Gst::State::STATE_NULL);
  std::cout << "Pipeline done" << std::endl;
}

void Pipeline::handle_message(Glib::RefPtr<Gst::Message> msg) {
  switch (msg->get_message_type()) {
  case Gst::MessageType::MESSAGE_ERROR: {
    auto err = Glib::RefPtr<Gst::MessageError>::cast_static(msg);
    std::cerr << "Error from " << msg->get_source()->get_name() << ": "
              << err->parse_error().what() << std::endl
              << "Debug information: " << err->parse_debug() << std::endl;
    terminate_ = true;
  } break;
  case Gst::MessageType::MESSAGE_EOS: {
    std::cout << "Reached end of stream" << std::endl;
    terminate_ = true;
  } break;
  case Gst::MessageType::MESSAGE_STATE_CHANGED: {
    if (msg->get_source() == pipeline_) {
      auto err = Glib::RefPtr<Gst::MessageStateChanged>::cast_static(msg);
      auto old_state = err->parse_old_state();
      auto new_state = err->parse_new_state();
      std::cout << "Pipeline state changed from " << old_state << " to "
                << new_state << std::endl;
      ready_ = new_state == Gst::State::STATE_READY;
      playing_ = new_state == Gst::State::STATE_PLAYING;
    }
  } break;
  default:
    std::cerr << "Unrecognized message type "
              << gst_message_type_get_name(
                     static_cast<GstMessageType>(msg->get_message_type()))
              << std::endl;
    break;
  }
}
