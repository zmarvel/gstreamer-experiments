
#include <iostream>

#include <gstreamermm.h>

int main(int argc, char *argv[]) {
  Gst::init(argc, argv);

  auto source = Gst::ElementFactory::create_element("videotestsrc", "source");
  auto sink = Gst::ElementFactory::create_element("autovideosink", "sink");

  auto pipeline = Gst::Pipeline::create("test-pipeline");

  // Add elements to the pipeline (a bin) and link them together.
  // Elements have to belong to the same bin to link together, and the linking
  // order is source->sink.
  try {
    pipeline->add(source)->add(sink);
    source->link(sink);
  } catch (std::runtime_error &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  source->set_property("pattern", 0);

  if (pipeline->set_state(Gst::State::STATE_PLAYING) ==
      Gst::StateChangeReturn::STATE_CHANGE_FAILURE) {
    std::cerr << "Unable to set pipeline state to PLAYING" << std::endl;
    return 1;
  }

  auto bus = pipeline->get_bus();
  auto msg = bus->pop(Gst::CLOCK_TIME_NONE, Gst::MessageType::MESSAGE_ERROR |
                                                Gst::MessageType::MESSAGE_EOS);

  if (msg) {
    switch (msg->get_message_type()) {
    case Gst::MessageType::MESSAGE_ERROR: {
      auto err = Glib::RefPtr<Gst::MessageError>::cast_static(msg);
      std::cerr << "Error from " << msg->get_source()->get_name() << ": "
                << err->parse_error().what() << std::endl
                << "Debug information: " << err->parse_debug() << std::endl;
    } break;
    case Gst::MessageType::MESSAGE_EOS: {
      auto err = Glib::RefPtr<Gst::MessageEos>::cast_static(msg);
      std::cout << "Reached end of stream" << std::endl;
    } break;
    default:
      break;
    }
  }

  pipeline->set_state(Gst::State::STATE_NULL);
}
