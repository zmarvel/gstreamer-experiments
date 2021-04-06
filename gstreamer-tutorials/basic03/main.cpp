
#include <iostream>

#include <gstreamermm.h>
#include <glibmm/main.h>

struct PipelineElements {
  Glib::RefPtr<Gst::Element> source;
  Glib::RefPtr<Gst::Element> convert;
  Glib::RefPtr<Gst::Element> resample;
  Glib::RefPtr<Gst::Element> sink;

  void register_pad_added_handler() {
    // I think we have to use a wrapper function in order to take the address of
    // the member function.
    source->signal_pad_added().connect(
        sigc::mem_fun(*this, &PipelineElements::pad_added_handler));
  }

private:
  void pad_added_handler(Glib::RefPtr<Gst::Pad> new_pad) {
    // Why is the first argument (source element) omitted? I guess we can assume
    // it's `source`.
    // If we wanted to reuse the same handler for signals from multiple sources,
    // we could just bind another argument I suppose.
    auto sink_pad = convert->get_static_pad("sink");

    std::cout << "Received new pad " << new_pad->get_name() << " from "
              << source->get_name() << std::endl;

    // Nothing to do if it's already linked
    if (!sink_pad->is_linked()) {
      auto new_pad_caps = new_pad->get_current_caps();
      auto new_pad_struct = new_pad_caps->get_structure(0);
      auto new_pad_type = new_pad_struct.get_name();
      std::string desired_type{"audio/x-raw"};
      if (new_pad_type.compare(0, desired_type.size(), desired_type) != 0) {
        std::cerr << "Pad has type " << new_pad_type << " but " << desired_type
                  << " is required" << std::endl;
        return;
      }

      if (new_pad->link(sink_pad) != Gst::PadLinkReturn::PAD_LINK_OK) {
        std::cerr << "Pad link failed" << std::endl;
      } else {
        std::cout << "Pad link succeess!" << std::endl;
      }
    }
  }
};

int main(int argc, char *argv[]) {
  Gst::init(argc, argv);

  PipelineElements elements{
      .source = Gst::ElementFactory::create_element("uridecodebin", "source"),
      .convert = Gst::ElementFactory::create_element("audioconvert", "convert"),
      .resample =
          Gst::ElementFactory::create_element("audioresample", "resample"),
      .sink = Gst::ElementFactory::create_element("autoaudiosink", "sink"),
  };

  auto pipeline = Gst::Pipeline::create("test-pipeline");

  // This logic could be moved to PipelineElements constructor
  try {
    pipeline->add(elements.source)
        ->add(elements.convert)
        ->add(elements.resample)
        ->add(elements.sink);
    elements.convert->link(elements.resample)->link(elements.sink);
  } catch (std::runtime_error &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  elements.source->set_property(
      "uri", std::string{"https://www.freedesktop.org/software/gstreamer-sdk/"
                         "data/media/sintel_trailer-480p.webm"});

  elements.register_pad_added_handler();

  if (pipeline->set_state(Gst::State::STATE_PLAYING) ==
      Gst::StateChangeReturn::STATE_CHANGE_FAILURE) {
    std::cerr << "Unable to set pipeline state to PLAYING" << std::endl;
    return 1;
  }

  auto bus = pipeline->get_bus();
  auto msg =
      bus->pop(Gst::CLOCK_TIME_NONE, Gst::MessageType::MESSAGE_STATE_CHANGED |
                                         Gst::MessageType::MESSAGE_ERROR |
                                         Gst::MessageType::MESSAGE_EOS);
  bool done = false;

  while (!done) {
    if (msg) {
      switch (msg->get_message_type()) {
      case Gst::MessageType::MESSAGE_STATE_CHANGED: {
        if (msg->get_source() == pipeline) {
          auto err = Glib::RefPtr<Gst::MessageStateChanged>::cast_static(msg);
          auto old_state = err->parse_old_state();
          auto new_state = err->parse_new_state();
          // auto pending_state = err->parse_pending_state();
          std::cout << "Pipeline state changed from " << old_state << " to "
                    << new_state << std::endl;
        }
      } break;
      case Gst::MessageType::MESSAGE_ERROR: {
        auto err = Glib::RefPtr<Gst::MessageError>::cast_static(msg);
        std::cerr << "Error from " << msg->get_source()->get_name() << ": "
                  << err->parse_error().what() << std::endl
                  << "Debug information: " << err->parse_debug() << std::endl;
      } break;
      case Gst::MessageType::MESSAGE_EOS: {
        // auto err = Glib::RefPtr<Gst::MessageEos>::cast_static(msg);
        std::cout << "Reached end of stream" << std::endl;
        done = true;
      } break;
      default:
        break;
      }
    }
  }

  pipeline->set_state(Gst::State::STATE_NULL);
}
