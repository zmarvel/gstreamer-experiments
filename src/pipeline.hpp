#pragma once
#include <iostream>
#include <gstreamermm.h>
#include <chrono>

#include "frame_parameters.hpp"
#include "frame.hpp"

namespace camcoder {

class Pipeline {
public:
  /**
   * Construct and set up the pipeline.
   */
  Pipeline(const FrameParameters &frame_params);

  template <typename TPixel>
  void push_frame(std::unique_ptr<FrameTmpl<TPixel>> pframe,
                  std::chrono::nanoseconds timestamp) {
    // copy the frame into a Gst::Buffer
    // emit push-buffer signal via Glib::SignalProxy
    auto framebuf = Gst::Buffer::create(pframe->size_bytes());
    framebuf->fill(0, reinterpret_cast<gconstpointer>(pframe->data()),
                   pframe->size_bytes());
    framebuf->set_dts(timestamp.count());
    // TODO: figure out glibmm SignalProxy
    // See gstreamermm/examples/media_player_getkmm/player_window.cc
    // for SignalProxy example
    GstFlowReturn ret = GST_FLOW_ERROR;
    g_signal_emit_by_name(appsrc_->gobj(), "push-buffer", framebuf->gobj(),
                          &ret);
    std::cout << "Emit buffer" << std::endl;
    if (ret < 0) {
      std::cerr << gst_flow_get_name(ret) << std::endl;
    }
  }

  void stop() {
    std::cout << "stop()" << std::endl;
    terminate_ = true;
  }

  bool playing() const { return playing_; }

  /**
   * Run the pipeline.
   */

  // TODO: is time point right?
  void operator()();

private:
  void handle_message(Glib::RefPtr<Gst::Message> msg);

  // This lets our app inject data into the pipeline, but we have to tell it
  // e.g. video format
  Glib::RefPtr<Gst::Element> appsrc_;

  // Convert format into something the encoder can use
  Glib::RefPtr<Gst::Element> convert_;
  // We'll use H264 for now
  Glib::RefPtr<Gst::Element> encoder_;
  Glib::RefPtr<Gst::Element> tsmux_;
  // We'll use HLS
  Glib::RefPtr<Gst::Element> hls_sink_;
  Glib::RefPtr<Gst::Pipeline> pipeline_;

  bool terminate_; /// True when the pipeline should be stopped
  bool playing_;   /// True if the pipeline is in the playing state
  bool ready_;
};
} // namespace camcoder
