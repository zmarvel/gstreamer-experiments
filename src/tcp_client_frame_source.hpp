#pragma once

#include <string_view>

#include <sockpp/tcp_connector.h>
#include <sockpp/tcp6_connector.h>

#include "frame_source.hpp"

namespace camcoder {

namespace detail {
template <typename TConnector = sockpp::tcp_connector>
class TCPClientFrameSource : public FrameSource {

  static_assert(std::is_same_v<TConnector, sockpp::tcp_connector> ||
                std::is_same_v<TConnector, sockpp::tcp6_connector>);

public:
  // TODO: Does this do hostname resolution?
  TCPClientFrameSource(const std::string &host, std::uint16_t port,
                       const FrameParameters &frame_params)
      : FrameSource{frame_params}, addr_{host, port}, connector_{addr_} {}

private:
  size_t read(char *buf, size_t n) override {
    if (!connected()) {
      return 0;
    }
    return connector_.read_n(buf, n);
  }
  bool eof() const override { return false; }
  bool good() const override { return connected_(); }
  bool bad() const override { return !connected_(); }

  bool connected_() const override { return connector_.is_connected(); }

  bool connect_() override {
    // if sockpp::connector.connect() is called twice, it will break the
    // connection and reconnect.
    if (connected()) {
      return true;
    } else {
      return connector_.connect(addr_);
    }
  }

  typename TConnector::addr_t addr_;
  TConnector connector_;
};
} // namespace detail

using TCPClientFrameSource =
    detail::TCPClientFrameSource<sockpp::tcp_connector>;
using TCP6ClientFrameSource =
    detail::TCPClientFrameSource<sockpp::tcp6_connector>;

} // namespace camcoder
