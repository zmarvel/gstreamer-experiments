#pragma once

#include <gstreamermm.h>
#include <chrono>

#include "frame_parameters.hpp"
#include "frame.hpp"
#include "config.hpp"
#include "frame_source.hpp"

namespace camcoder {

class Pipeline {
public:
  /**
   * Construct and set up the pipeline.
   */
  Pipeline(const Config &config);

  void stop() { terminate_ = true; }

  bool playing() const { return playing_; }

  void add_frame_source(std::unique_ptr<FrameThread> frame_source);

  /**
   * Run the pipeline.
   */
  void operator()();

private:
  void handle_message(Glib::RefPtr<Gst::Message> msg);

  static void appsrc_need_data_callback(GstElement *appsrc, guint length,
                                        gpointer udata);

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
  std::vector<std::unique_ptr<FrameThread>> frame_sources_;
};
} // namespace camcoder
