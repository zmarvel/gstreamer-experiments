
#include <gstreamermm.h>

int main(int argc, char* argv[]) {
  Gst::init(argc, argv);

  auto pipeline = Gst::Pipeline::create("my-pipeline");

  auto playbin = Glib::RefPtr<Gst::Bin>::cast_static(Gst::ElementFactory::create_element("playbin"));
  playbin->set_property("uri", std::string{"https://www.freedesktop.org/software/gstreamer-sdk/data/media/sintel_trailer-480p.webm"});

  pipeline->add(playbin);
  pipeline->set_state(Gst::State::STATE_PLAYING);

  auto bus = pipeline->get_bus();
  auto msg = bus->pop(Gst::CLOCK_TIME_NONE, Gst::MessageType::MESSAGE_ERROR | Gst::MessageType::MESSAGE_EOS);

  pipeline->set_state(Gst::State::STATE_NULL);
}
