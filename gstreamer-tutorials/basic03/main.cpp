
#include <iostream>

#include <gstreamermm.h>
#include <glibmm/main.h>

struct PipelineElements {
  Glib::RefPtr<Gst::Element> source;
  Glib::RefPtr<Gst::Element> audio_convert;
  Glib::RefPtr<Gst::Element> audio_resample;
  Glib::RefPtr<Gst::Element> audio_sink;
  Glib::RefPtr<Gst::Element> video_convert;
  Glib::RefPtr<Gst::Element> video_sink;

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
    auto audio_sink_pad = audio_convert->get_static_pad("sink");
    auto video_sink_pad = video_convert->get_static_pad("sink");

    auto new_pad_caps = new_pad->get_current_caps();
    auto new_pad_struct = new_pad_caps->get_structure(0);
    auto new_pad_type = new_pad_struct.get_name();

    std::cout << "Received new pad " << new_pad->get_name() << " with type "
              << new_pad_type << " from " << source->get_name() << std::endl;

    if (!audio_sink_pad->is_linked()) {
      const std::string desired_type{"audio/x-raw"};
      if (new_pad_type.compare(0, desired_type.size(), desired_type) != 0) {
        std::cerr << "Pad has type " << new_pad_type << " but " << desired_type
                  << " is required" << std::endl;
      } else {
        if (new_pad->link(audio_sink_pad) != Gst::PadLinkReturn::PAD_LINK_OK) {
          std::cerr << "Audio sink pad link failed" << std::endl;
        } else {
          std::cout << "Audio sink pad link succeess!" << std::endl;
        }
      }
    }

    if (!video_sink_pad->is_linked()) {
      const std::string desired_type{"video/x-raw"};
      if (new_pad_type.compare(0, desired_type.size(), desired_type) != 0) {
        std::cerr << "Pad has type " << new_pad_type << " but " << desired_type
                  << " is required" << std::endl;
      } else {
        if (new_pad->link(video_sink_pad) != Gst::PadLinkReturn::PAD_LINK_OK) {
          std::cerr << "Video sink pad link failed" << std::endl;
        } else {
          std::cout << "Video sink pad link succeess!" << std::endl;
        }
      }
    }
  }
};

int main(int argc, char *argv[]) {
  Gst::init(argc, argv);

  PipelineElements elements{
      .source = Gst::ElementFactory::create_element("uridecodebin", "source"),
      .audio_convert =
          Gst::ElementFactory::create_element("audioconvert", "audio-convert"),
      .audio_resample = Gst::ElementFactory::create_element("audioresample",
                                                            "audio-resample"),
      .audio_sink =
          Gst::ElementFactory::create_element("autoaudiosink", "audio-sink"),
      .video_convert =
          Gst::ElementFactory::create_element("videoconvert", "video-convert"),
      .video_sink =
          Gst::ElementFactory::create_element("autovideosink", "video-sink"),
  };

  auto pipeline = Gst::Pipeline::create("test-pipeline");

  // This logic could be moved to PipelineElements constructor
  try {
    pipeline->add(elements.source)
        ->add(elements.audio_convert)
        ->add(elements.audio_resample)
        ->add(elements.audio_sink)
        ->add(elements.video_convert)
        ->add(elements.video_sink);
    elements.audio_convert->link(elements.audio_resample)
        ->link(elements.audio_sink);
    elements.video_convert->link(elements.video_sink);
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
