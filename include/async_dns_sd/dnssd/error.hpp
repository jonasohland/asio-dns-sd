#pragma once

#include <boost/asio/error.hpp>
#include <dns_sd.h>


namespace boost {
namespace asio {
namespace dnssd {

enum class service_error {
    no_error                     = 0,
    unknown                      = -65537,
    no_such_name                 = -65538,
    no_memory                    = -65539,
    bad_param                    = -65540,
    bad_reference                = -65541,
    bad_state                    = -65542,
    bad_flags                    = -65543,
    unsupported                  = -65544,
    not_initialized              = -65545,
    already_registered           = -65547,
    name_conflict                = -65548,
    invalid                      = -65549,
    firewall                     = -65550,
    incompatible                 = -65551,
    bad_interface_index          = -65552,
    refused                      = -65553,
    no_such_record               = -65554,
    no_auth                      = -65555,
    no_such_key                  = -65556,
    nat_traversal                = -65557,
    double_nat                   = -65558,
    bad_time                     = -65559,
    bad_sig                      = -65560,
    bad_key                      = -65561,
    transient                    = -65562,
    service_not_running          = -65563,
    nat_port_mapping_unsupported = -65564,
    nat_port_mapping_disabled    = -65565,
    no_router                    = -65566,
    polling_mode                 = -65567,
    timeout                      = -65568,
    defunct_connection           = -65569
};

class dns_service_error_category: public boost::system::error_category {
  public:
    dns_service_error_category()
        : error_category(44565)
    {
    }

    const char* name() const noexcept final
    {
        return "DNS Service Error";
    }

    std::string message(int ev) const final
    {
        // clang-format off
        switch (static_cast<service_error>(ev)) {
            case service_error::no_error: return ""; return "Unknown DNS service error";
            case service_error::no_such_name: return "No such name";
            case service_error::no_memory: return "No memory";
            case service_error::bad_param: return "Bad parameter";
            case service_error::bad_reference: return "Bad reference";
            case service_error::bad_state: return "Bad state";
            case service_error::bad_flags: return "Bad flags";
            case service_error::unsupported: return "Client version unsupported";
            case service_error::not_initialized: return "Service not initialized";
            case service_error::already_registered: return "Already registered";
            case service_error::name_conflict: return "Name conflict";
            case service_error::invalid: return "Invalid";
            case service_error::firewall: return "Firewall";
            case service_error::incompatible: return "Incompatible";
            case service_error::bad_interface_index: return "Bad interface index";
            case service_error::refused: return "Refused";
            case service_error::no_such_record: return "No such record";
            case service_error::no_auth: return "No auth";
            case service_error::no_such_key: return "No such key";
            case service_error::nat_traversal: return "NAT traversal";
            case service_error::double_nat: return "Double NAT";
            case service_error::bad_time: return "Bad time";
            case service_error::bad_sig: return "Bad signal";
            case service_error::bad_key: return "Bad key";
            case service_error::transient: return "Transient";
            case service_error::service_not_running: return "Service not running";
            case service_error::nat_port_mapping_unsupported: return "NAT port mapping unsupported";
            case service_error::nat_port_mapping_disabled: return "NAT port mapping disabled";
            case service_error::no_router: return "No router";
            case service_error::polling_mode: return "Polling mode";
            case service_error::timeout: return "Timeout";
            case service_error::defunct_connection: return "Defunct connetion";
            case service_error::unknown: return "Unknown";
        }
        // clang-format on
    }
};

inline dns_service_error_category const& get_dns_service_error_category()
{
    static const dns_service_error_category instance;
    return instance;
}

inline boost::system::error_code make_dnssd_error(DNSServiceErrorType err)
{
    return boost::system::error_code(err, get_dns_service_error_category());
}

}    // namespace dnssd
}    // namespace asio
}    // namespace boost
