
#include <iostream>
#include <chrono>

#include <gstreamermm.h>
#include <glibmm/main.h>

struct Pipeline {
  bool playing;   /// Are we playing?
  bool terminate; /// Are we ready to exit?
  bool seek_enabled;
  bool seek_done;
  std::chrono::nanoseconds duration;

  Glib::RefPtr<Gst::Element> playbin;

  Pipeline()
      : playing{false}, terminate{false},
        seek_enabled{false}, seek_done{false}, duration{GST_CLOCK_TIME_NONE},
        playbin{Gst::ElementFactory::create_element("playbin", "playbin")} {
    playbin->set_property(
        "uri", std::string{"https://www.freedesktop.org/software/gstreamer-sdk/"
                           "data/media/sintel_trailer-480p.webm"});
  }

  void run() {
    if (playbin->set_state(Gst::State::STATE_PLAYING) ==
        Gst::StateChangeReturn::STATE_CHANGE_FAILURE) {
      throw std::runtime_error{"Failed to change to playing state"};
    }

    auto bus = playbin->get_bus();
    while (!terminate) {
      auto msg = bus->pop(100 * Gst::MILLI_SECOND,
                          Gst::MessageType::MESSAGE_STATE_CHANGED |
                              Gst::MessageType::MESSAGE_ERROR |
                              Gst::MessageType::MESSAGE_EOS |
                              Gst::MessageType::MESSAGE_DURATION_CHANGED);

      if (msg) {
        handle_message(msg);
      } else {
        // The timeout expired
        if (playing) {
          gint64 current = -1;
          if (!playbin->query_position(Gst::Format::FORMAT_TIME, current)) {
            std::cerr << "Failed to query current position" << std::endl;
          }

          if (GST_CLOCK_TIME_IS_VALID(duration.count())) {
            gint64 duration_ns = GST_CLOCK_TIME_NONE;
            if (!playbin->query_duration(Gst::Format::FORMAT_TIME,
                                         duration_ns)) {
              std::cerr << "Failed to query duration" << std::endl;
            } else {
              duration = decltype(duration){duration_ns};
            }
          }

          std::cout << "Position " << current << " / " << duration.count()
                    << std::endl;
          if (seek_enabled && !seek_done && current > 10 * GST_SECOND) {
            std::cout << "Reached 10 s, performing seek" << std::endl;
            playbin->seek(Gst::Format::FORMAT_TIME,
                          Gst::SeekFlags::SEEK_FLAG_FLUSH |
                              Gst::SeekFlags::SEEK_FLAG_KEY_UNIT,
                          30 * GST_SECOND);
            seek_done = true;
          }
        }
      }
    }

    playbin->set_state(Gst::State::STATE_NULL);
  }

private:
  void handle_message(Glib::RefPtr<Gst::Message> msg) {
    switch (msg->get_message_type()) {
    case Gst::MessageType::MESSAGE_ERROR: {
      auto err = Glib::RefPtr<Gst::MessageError>::cast_static(msg);
      std::cerr << "Error from " << msg->get_source()->get_name() << ": "
                << err->parse_error().what() << std::endl
                << "Debug information: " << err->parse_debug() << std::endl;
      terminate = true;
    } break;
    case Gst::MessageType::MESSAGE_EOS: {
      // auto err = Glib::RefPtr<Gst::MessageEos>::cast_static(msg);
      std::cout << "Reached end of stream" << std::endl;
      terminate = true;
    } break;
    case Gst::MessageType::MESSAGE_STATE_CHANGED: {
      if (msg->get_source() == playbin) {
        auto err = Glib::RefPtr<Gst::MessageStateChanged>::cast_static(msg);
        auto old_state = err->parse_old_state();
        auto new_state = err->parse_new_state();
        // auto pending_state = err->parse_pending_state();
        std::cout << "Pipeline state changed from " << old_state << " to "
                  << new_state << std::endl;
        if (new_state == Gst::State::STATE_PLAYING) {
          playing = true;
          // Query the element to determine if we can seek
          gint64 seek_start = -1;
          gint64 seek_end = -1;
#if 0
          // I think this should work, but it doesn't. See
          // gstreamermm/examples/ogg_player/main.cc and the comment in
          // gstreamermm/query.hg.
          Glib::RefPtr<Gst::QuerySeeking> query =
              Gst::QuerySeeking::create(Gst::Format::FORMAT_TIME);
          query->parse(format, seek_enabled, seek_start, seek_end);
#endif
          Glib::RefPtr<Gst::Query> query =
              Gst::QuerySeeking::create(Gst::Format::FORMAT_TIME);

          if (playbin->query(query)) {
            auto format = Gst::Format::FORMAT_TIME;
            Glib::RefPtr<Gst::QuerySeeking>::cast_static(query)->parse(
                format, seek_enabled, seek_start, seek_end);
            if (seek_enabled) {
              std::cout << "Seeking is enabled from " << seek_start << " to "
                        << seek_end << std::endl;
            } else {
              std::cout << "Seeking is disabled for playbin" << std::endl;
            }
          } else {
            std::cerr << "Failed to perform seeking query on playbin"
                      << std::endl;
          }
        } else {
          playing = false;
        }
      }
    } break;
    default:
      break;
    }
  }
};

int main(int argc, char *argv[]) {
  Gst::init(argc, argv);

  Pipeline pipeline{};
  pipeline.run();

  pipeline.run();
}
