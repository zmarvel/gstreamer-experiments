#include <spdlog/spdlog.h>

#include "pipeline.hpp"
#include "utils.hpp"

using namespace camcoder;

Pipeline::Pipeline(const Config &config)
    : convert_{Gst::ElementFactory::create_element("videoconvert")},
      encoder_{Gst::ElementFactory::create_element("x264enc")},
      // encoder_{Gst::ElementFactory::create_element("vaapih264enc")},
      // encoder_{Gst::ElementFactory::create_element("nvh264enc")},
      tsmux_{Gst::ElementFactory::create_element("mpegtsmux")},
      hls_sink_{Gst::ElementFactory::create_element("hlssink")},
      pipeline_{Gst::Pipeline::create()},
      terminate_{false}, playing_{false}, ready_{false} {

  // TODO: check for null elements
  hls_sink_->set_property(
      "location", utils::path_join(config.output_directory, "segment%05d.ts"));
  hls_sink_->set_property(
      "playlist-location",
      utils::path_join(config.output_directory, "playlist.m3u8"));

  pipeline_->add(convert_)->add(encoder_)->add(tsmux_)->add(hls_sink_);
  convert_->link(encoder_)->link(tsmux_)->link(hls_sink_);
}

void Pipeline::operator()() {
  if (pipeline_->set_state(Gst::State::STATE_PLAYING) ==
      Gst::StateChangeReturn::STATE_CHANGE_FAILURE) {
    throw std::runtime_error{"Failed to change to playing state"};
  }
  spdlog::info("Pipeline starting");

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
  spdlog::info("Pipeline done");
}

void Pipeline::add_frame_source(std::unique_ptr<FrameThread> frame_source) {
  auto appsrc = Gst::ElementFactory::create_element("appsrc");

  Gst::VideoInfo video_info;
  video_info.init();
  const auto &frame_params = frame_source->frame_parameters();
  video_info.set_format(Gst::VideoFormat::VIDEO_FORMAT_RGB, frame_params.width,
                        frame_params.height);
  const auto frame_rate = frame_source->frame_rate();
  if (frame_rate.numerator > 0) {
    video_info.set_fps_n(frame_rate.numerator);
    video_info.set_fps_d(frame_rate.denominator);
  }
  spdlog::info("Using frame rate {}/{}", frame_rate.numerator,
               frame_rate.denominator);
  auto video_caps = video_info.to_caps();

  appsrc->set_property("caps", video_caps);
  appsrc->set_property("block", true);

  g_signal_connect(appsrc->gobj(), "need-data",
                   G_CALLBACK(appsrc_need_data_callback),
                   reinterpret_cast<gpointer>(frame_source.get()));
  pipeline_->add(appsrc);
  appsrc->link(convert_);
  frame_sources_.push_back(std::move(frame_source));
}

void Pipeline::handle_message(Glib::RefPtr<Gst::Message> msg) {
  switch (msg->get_message_type()) {
  case Gst::MessageType::MESSAGE_ERROR: {
    auto err = Glib::RefPtr<Gst::MessageError>::cast_static(msg);
    spdlog::error("Error from {}:", msg->get_source()->get_name().raw());
    spdlog::error("Debug information: {}", err->parse_debug());
    terminate_ = true;
  } break;
  case Gst::MessageType::MESSAGE_EOS: {
    spdlog::info("Reached end of stream");
    terminate_ = true;
  } break;
  case Gst::MessageType::MESSAGE_STATE_CHANGED: {
    if (msg->get_source() == pipeline_) {
      auto err = Glib::RefPtr<Gst::MessageStateChanged>::cast_static(msg);
      auto old_state = err->parse_old_state();
      auto new_state = err->parse_new_state();
      spdlog::debug("Pipeline state changed from {} to {}", old_state,
                    new_state);
      ready_ = new_state == Gst::State::STATE_READY;
      playing_ = new_state == Gst::State::STATE_PLAYING;
    }
  } break;
  default:
    spdlog::error("Unrecognized message type {}",
                  gst_message_type_get_name(
                      static_cast<GstMessageType>(msg->get_message_type())));
    break;
  }
}

void Pipeline::appsrc_need_data_callback(GstElement *appsrc, guint length,
                                         gpointer udata) {
  auto frame_source = reinterpret_cast<FrameThread *>(udata);
  auto pframe = frame_source->pop_frame();

  auto framebuf = Gst::Buffer::create(pframe->size_bytes());
  framebuf->fill(0, pframe->raw_data(), pframe->size_bytes());
  const auto frame_rate = frame_source->frame_rate();
  framebuf->set_duration(frame_rate.denominator * 1e9 / frame_rate.numerator);
  const auto timestamp = pframe->timestamp().count();
  if (timestamp == 0) {
    framebuf->set_dts(pframe->frame_number() *
                      (frame_rate.denominator * 1e9 / frame_rate.numerator));
  } else {
    framebuf->set_dts(timestamp);
  }

  // TODO: figure out glibmm SignalProxy
  // See gstreamermm/examples/media_player_getkmm/player_window.cc
  // for SignalProxy example
  GstFlowReturn ret = GST_FLOW_ERROR;
  g_signal_emit_by_name(appsrc, "push-buffer", framebuf->gobj(), &ret);
  if (ret < 0) {
    spdlog::error(gst_flow_get_name(ret));
  }
}
