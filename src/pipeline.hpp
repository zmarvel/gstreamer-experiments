#ifndef CAMCODER
#define CAMCODER

#include <gstreamermm.h>

namespace camcoder {
class Pipeline {
public:
  /**
   * Construct and set up the pipeline.
   */
  Pipeline();

  /**
   * Run the pipeline.
   */
  void operator()();

private:
  void handle_message(Glib::RefPtr<Gst::Message> msg);

  Glib::RefPtr<Gst::Pipeline> pipeline_;
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

  bool terminate_; /// True when the pipeline should be stopped
  bool playing_;   /// True if the pipeline is in the playing state
};
} // namespace camcoder

#endif // CAMCODER
