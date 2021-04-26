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

  static std::unique_ptr<TCPClientFrameSource>
  from_config(const FrameSourceConfig &config) {
    const auto host = config.options.find("host");
    std::string addr_host{};
    if (host == config.options.end()) {
      spdlog::warn("No host specified for source {}; using 127.0.0.1",
                   config.name);
      addr_host = "127.0.0.1";
    } else {
      addr_host = host->second.as_string();
    }

    const auto port = config.options.find("port");
    if (port == config.options.end()) {
      spdlog::error("Port is required for source {}", config.name);
      return nullptr;
    }
    const auto addr_port = port->second.as_integer();
    if (addr_port <= 0 ||
        addr_port > std::numeric_limits<std::uint16_t>::max()) {
      spdlog::error("Invalid port {} for source {}", addr_port, config.name);
      return nullptr;
    }

    spdlog::info("Creating TCPClientFrameSource<host={}, port={}>", addr_host,
                 addr_port);

    return std::make_unique<TCPClientFrameSource>(
        addr_host, static_cast<std::uint16_t>(addr_port), config.frame_params);
  }

  std::string host() const { return addr_.to_string(); }
  std::uint16_t port() const { return addr_.port(); }

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
