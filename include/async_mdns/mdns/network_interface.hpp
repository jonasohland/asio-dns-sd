#pragma once

#include "util.hpp"

namespace boost {
    namespace asio {
        namespace mdns {


            class network_interface {

              public:
                network_interface(const ip::address& ip_addr)
                    : addr_(ip_addr)
                {
                }

                network_interface(const ip::address& ip_addr,
                                  unsigned int index)
                    : index_(index)
                    , addr_(ip_addr)
                {
                }

                network_interface(unsigned int index)
                    : index_(index)
                {
                }

                network_interface(const std::string& if_name)
                {
                }

                ip::address ip_address()
                {
                    if (!ip_address_resolved())
                        resolve_ip(resolve_hint::use_any);

                    return addr_;
                }

                unsigned int index()
                {
                    if (!index_resolved())
                        resolve_index(resolve_hint::use_any);

                    return index_;
                }

                std::string name()
                {
                    if (!name_resolved())
                        resolve_name(resolve_hint::use_any);

                    return name_;
                }

                bool index_resolved() const
                {
                    return index_ != -1;
                }

                bool ip_address_resolved() const
                {
                    return !addr_.is_unspecified();
                }

                bool name_resolved() const
                {
                    return !name_.empty();
                }

                bool is_v4()
                {
                    if (!ip_address_resolved())
                        resolve_ip(resolve_hint::use_any);

                    return addr_.is_v4();
                }

                bool is_v6()
                {
                    if (!ip_address_resolved())
                        resolve_ip(resolve_hint::use_any);

                    return addr_.is_v6();
                }

                ip::tcp::endpoint tcp_endpoint(short port)
                {
                    if (is_v6())
                        return ip::tcp::endpoint(ip::tcp::v6(), port);
                    else
                        return ip::tcp::endpoint(ip::tcp::v4(), port);
                }

                ip::udp::endpoint udp_endpoint(short port)
                {
                    if (is_v6())
                        return ip::udp::endpoint(ip::udp::v6(), port);
                    else
                        return ip::udp::endpoint(ip::udp::v4(), port);
                }

                network_interface v4_version()
                {
                }

                network_interface v6_version()
                {
                }

              private:
                
                enum class resolve_hint {
                    use_any,
                    use_ipaddr,
                    use_name,
                    use_index
                };
                
                void resolve_ip(resolve_hint hint)
                {
                    if (ip_address_resolved())
                        return;
                }

                void resolve_index(resolve_hint hint)
                {
                    if (index_resolved())
                        return;
                    
                    if (!name_resolved() && !ip_address_resolved()) {
                        
                    }
                    
                    if (name_resolved()) {
                        index_ = ::if_nametoindex(name_.c_str());
                        return;
                    }
                }

                void resolve_name(resolve_hint hint)
                {
                    if (name_resolved())
                        return;

                    if (!index_resolved() && ip_address_resolved())
                        resolve_index(resolve_hint::use_ipaddr);

                    if (index_resolved()) {
                        char buf[IF_NAMESIZE];
                        ::if_indextoname(index_, buf);
                        name_.assign(buf);
                    }
                }

                unsigned int index_ = -1;
                std::string name_;
                ip::address addr_;
            };
        }    // namespace mdns
    }        // namespace asio
}    // namespace boost
