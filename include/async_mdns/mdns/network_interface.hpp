#pragma once

#include "util.hpp"

#ifndef _WIN32
namespace boost {
    namespace asio {
        namespace mdns {


            class network_interface {

              public:
                
                static network_interface all()
                {
                    return network_interface(0);
                }
                
                using index_type = unsigned int;
                
                explicit network_interface(const ip::address& ip_addr)
                    : addr_(ip_addr)
                {
                }

                explicit network_interface(const ip::address& ip_addr,
                                  unsigned int index)
                    : index_(index)
                    , addr_(ip_addr)
                {
                }

                explicit network_interface(index_type index)
                    : index_(index)
                {
                }

                explicit network_interface(const std::string& if_name)
                    : name_(if_name)
                {
                }

                ip::address ip_address()
                {
                    if (!ip_address_resolved())
                        resolve_ip();

                    return addr_;
                }

                index_type index()
                {
                    if (!index_resolved())
                        resolve_index();

                    return index_;
                }

                std::string name()
                {
                    if (!name_resolved())
                        resolve_name();

                    return name_;
                }

                bool index_resolved() const
                {
                    return index_;
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
                        resolve_ip();

                    return addr_.is_v4();
                }

                bool is_v6()
                {
                    if (!ip_address_resolved())
                        resolve_ip();

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

              private:
                
                enum class resolve_hint {
                    use_any,
                    use_ipaddr,
                    use_name,
                    use_index
                };
                
                void resolve_ip()
                {
                    if (ip_address_resolved())
                        return;
                    
                    if (!name_resolved() && index_resolved()) {
                        
                    } else if (name_resolved()) {
                        
                    }
                }

                void resolve_index()
                {
                    if (index_resolved())
                        return;
                    
                    if (!name_resolved() && ip_address_resolved()) {
                        resolve_name();
                    }
                    
                    if (name_resolved()) {
#ifdef _WIN32
                        
#else
                        index_ = ::if_nametoindex(name_.c_str());
#endif
                        return;
                    }
                }

                void resolve_name()
                {
                    if (name_resolved())
                        return;

                    if (!index_resolved() && ip_address_resolved())
                        resolve_index();

                    if (index_resolved()) {
                        char buf[IF_NAMESIZE];
                        ::if_indextoname(index_, buf);
                        name_.assign(buf);
                    }
                }
                
                ::sockaddr* find_addr_by_index(ifaddrs* current, unsigned int index)
                {
                    char name[IF_NAMESIZE];
                    ::if_indextoname(index, name);
                    return find_addr_by_name(current, name);
                }
                
                ::sockaddr* find_addr_by_name(ifaddrs* current, const char* ifname)
                {
                    if (!current)
                        return nullptr;
                    
                    if (::strcmp(current->ifa_name, ifname) == 0)
                        return current->ifa_addr;
                    
                    return find_addr_by_name(current->ifa_next, ifname);
                }
            
                index_type index_ = 0;
                std::string name_;
                ip::address addr_;
            };
        }    // namespace mdns
    }        // namespace asio
}    // namespace boost
#endif
