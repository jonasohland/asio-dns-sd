#pragma once

#include "service_record.hpp"
#include "transport_names.hpp"

namespace boost {
namespace asio {
namespace dnssd {

template <typename TransportProtocol>
class basic_service {
  public:
    class type {
      public:
        type(const std::string& protocol)
            : proto_(protocol)
        {
        }

        type(const std::string& protocol, const std::string& subprotocol)
            : proto_(protocol)
            , subproto_(subprotocol)
        {
        }

        std::string transport_name() const noexcept
        {
            return detail::transport_protocol_name<TransportProtocol>::value;
        }

        std::string service_type_string() const
        {
            auto s = proto_ + "." + transport_name();
            if (!subproto_.empty())
                s = s + "," + subproto_;
            return s;
        }

      private:
        std::string proto_;
        std::string subproto_;
    };

    using endpoint_type = typename TransportProtocol::endpoint;

    basic_service(basic_service<TransportProtocol>&& other)
        : associated_record_(std::move(other.associated_record_))
    {
    }

    basic_service(const basic_service<TransportProtocol>& other) = delete;

  private:
    service_record associated_record_;
};

using tcp_service = basic_service<boost::asio::ip::tcp>;
using udp_service = basic_service<boost::asio::ip::udp>;
}    // namespace dnssd
}    // namespace asio
}    // namespace boost
