#pragma once

#include <sockpp/tcp_acceptor.h>
#include <sockpp/tcp6_acceptor.h>

#include "frame_source.hpp"
#include "config.hpp"

namespace camcoder {

namespace detail {
template <typename TAcceptor = sockpp::tcp_acceptor>
class TCPServerFrameSource : public FrameSource {

  static_assert(std::is_same_v<TAcceptor, sockpp::tcp_acceptor> ||
                std::is_same_v<TAcceptor, sockpp::tcp6_acceptor>);

public:
  TCPServerFrameSource(const std::string &addr, std::uint16_t port,
                       const FrameParameters &frame_params,
                       const FrameRate &frame_rate = {0, 1})
      : FrameSource{frame_params, frame_rate}, addr_{addr, port},
        acceptor_{addr_}, client_sock_{}, client_addr_{} {}
  // TODO: constructor without specific address

  static std::unique_ptr<TCPServerFrameSource>
  from_config(const FrameSourceConfig &config) {
    const auto host = config.options.find("host");
    std::string addr_host;
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

    spdlog::info("Creating TCPServerFrameSource<host={}, port={}>", addr_host,
                 addr_port);

    return std::make_unique<TCPServerFrameSource>(
        addr_host, static_cast<std::uint16_t>(addr_port), config.frame_params,
        config.frame_rate);
  }

private:
  size_t read(char *buf, size_t n) override {
    if (!connected()) {
      return 0;
    } else {
      return client_sock_.read_n(buf, n);
    }
  }
  bool eof() const override { return false; }
  bool good() const override { return connected_(); }
  bool bad() const override { return !connected_(); }

  bool connected_() const override { return client_sock_.is_open(); }
  bool connect_() override {
    client_sock_ = acceptor_.accept(&client_addr_);
    return client_sock_.is_open();
  }

private:
  typename TAcceptor::addr_t addr_;
  TAcceptor acceptor_;
  sockpp::stream_socket client_sock_;
  typename TAcceptor::addr_t client_addr_;
};
} // namespace detail

using TCPServerFrameSource = detail::TCPServerFrameSource<sockpp::tcp_acceptor>;
using TCP6ServerFrameSource =
    detail::TCPServerFrameSource<sockpp::tcp6_acceptor>;

} // namespace camcoder
