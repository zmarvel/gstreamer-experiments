
#include <iostream>

#include "pipeline.hpp"

using namespace camcoder;

Pipeline::Pipeline()
    : pipeline_{Gst::Pipeline::create()},
      appsrc_{Gst::ElementFactory::create_element("videotestsrc")},
      convert_{Gst::ElementFactory::create_element("videoconvert")},
      encoder_{Gst::ElementFactory::create_element("vaapih264enc")},
      // encoder_{Gst::ElementFactory::create_element("nvh264enc")},
      tsmux_{Gst::ElementFactory::create_element("mpegtsmux")},
      hls_sink_{Gst::ElementFactory::create_element("hlssink")},
      terminate_{false}, playing_{false} {
  // TODO for a true appsrc bin, we'll have to specify the format properties
  pipeline_->add(appsrc_)->add(convert_)->add(encoder_)->add(tsmux_)->add(
      hls_sink_);
  appsrc_->link(convert_)->link(encoder_)->link(tsmux_)->link(hls_sink_);

  auto test_src_pad = appsrc_->get_static_pad("src");
  test_src_pad->set_property("width", 640);
  test_src_pad->set_property("height", 480);
}

void Pipeline::operator()() {

  if (pipeline_->set_state(Gst::State::STATE_PLAYING) ==
      Gst::StateChangeReturn::STATE_CHANGE_FAILURE) {
    throw std::runtime_error{"Failed to change to playing state"};
  }

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
      if (playing_) {
      }
    }
  }
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
      if (new_state == Gst::State::STATE_PLAYING) {
        playing_ = true;
      } else {
        playing_ = false;
      }
    }
  } break;
  default:
    break;
  }
}
