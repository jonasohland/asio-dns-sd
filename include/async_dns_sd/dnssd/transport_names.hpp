#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

namespace boost {
namespace asio {
namespace dnssd {
namespace detail {
template <typename TransportProtocol>
struct transport_protocol_name {
};

template <>
struct transport_protocol_name<ip::tcp> {
    static constexpr const char* value = "_tcp";
};

template <>
struct transport_protocol_name<ip::udp> {
    static constexpr const char* value = "_udp";
};
}    // namespace detail
}    // namespace dnssd
}    // namespace asio
}    // namespace boost
