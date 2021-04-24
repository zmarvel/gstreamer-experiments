#pragma once

#include <sockpp/tcp_acceptor.h>
#include <sockpp/tcp6_acceptor.h>
#include "frame_source.hpp"

namespace camcoder {

namespace detail {
template <typename TAcceptor = sockpp::tcp_acceptor>
class TCPServerFrameSource : public FrameSource {
public:
  TCPServerFrameSource(const std::string &addr, std::uint16_t port,
                       const FrameParameters &frame_params)
      : FrameSource{frame_params}, addr_{addr, port}, acceptor_{addr_},
        client_sock_{}, client_addr_{} {}
  // TODO: constructor without specific address

  bool connected() const { return client_sock_.is_open(); }
  bool accept() {
    client_sock_ = acceptor_.accept(&client_addr_);
    return client_sock_.is_open();
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
  bool good() const override { return connected(); }
  bool bad() const override { return !connected(); }

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
