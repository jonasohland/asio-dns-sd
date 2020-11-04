#pragma once

#include "network_interface.hpp"
#include <boost/asio.hpp>

namespace boost {
    namespace asio {
        namespace mdns {

            class service_record {
              public:
                template <typename Executor>
                friend class basic_browser;

                service_record(unsigned int interface, const std::string& name,
                               const std::string& type,
                               const std::string& domain)
                    : name_(name)
                    , type_(type)
                    , domain_(domain)
                    , interface_(interface)
                {
                }

                std::string name() const
                {
                    return name_;
                }

                std::string type() const
                {
                    return type_;
                }

                std::string domain() const
                {
                    return domain_;
                }

                network_interface interface() const
                {
                    return network_interface(interface_);
                }

                std::string fqdn() const
                {
                    char bf[kDNSServiceMaxDomainName];
                    DNSServiceConstructFullName(
                        bf, name_.c_str(), type_.c_str(), domain_.c_str());
                    std::string out(bf);
                    return out;
                }


              private:
                std::string name_;
                std::string type_;
                std::string domain_;
                unsigned int interface_;
            };

        }    // namespace mdns
    }        // namespace asio
}    // namespace boost
