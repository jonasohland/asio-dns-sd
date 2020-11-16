#pragma once

#include "util.hpp"

#ifndef _WIN32
namespace boost {
    namespace asio {
        namespace mdns {

            class network_interface {

              public:
                using index_type          = unsigned int;
                using native_address_type = unsigned long;

                static network_interface all()
                {
                    return network_interface(0);
                }
                
                static std::vector<network_interface> list()
                {
                    std::vector<network_interface> ifs;
                    struct if_nameindex *if_ni, *i;
    
                    if_ni = ::if_nameindex();
                    
                    if (if_ni == 0)
                        return ifs;
                    
                    for (i = if_ni; ! (i->if_index == 0 && i->if_name == NULL); i++)
                        ifs.push_back(network_interface::from_ifnameindex(i));
                    
                    return ifs;
                }
                
                static network_interface from_ifnameindex(struct if_nameindex* if_ni)
                {
                    return network_interface(if_ni->if_index, if_ni->if_name);
                }
                
                explicit network_interface(index_type index, const std::string& name)
                    :index_(index), name_(name)
                {
                    fill_addr_from_ifname(name.c_str(), addresses_);
                }

                explicit network_interface(const ip::address& ip_addr)
                {
                    addresses_.push_back(ip_addr);
                    
                    ::ifaddrs* addrs;
                    ::getifaddrs(&addrs);
                    
                    name_ = find_name_by_addr(addrs, ip_addr);
                    
                    if (!name_.empty()) {
                        get_index_from_name();
                        fill_addr_from_ifname(name_.c_str(), addresses_);
                    }
                }

                explicit network_interface(index_type index)
                    : index_(index)
                {
                    fill_name_from_index();
                    if (name_specified())
                        fill_addr_from_ifname(name_.c_str(), addresses_);
                }

                explicit network_interface(const std::string& if_name)
                    : name_(if_name)
                {
                    get_index_from_name();
                    if (name_specified())
                        fill_addr_from_ifname(name_.c_str(), addresses_);
                }

                const std::vector<ip::address>& ip_addresses() const
                {
                    return addresses_;
                }

                index_type index() const
                {
                    return index_;
                }

                std::string name() const
                {
                    return name_;
                }

                bool index_resolved() const
                {
                    return index_;
                }

                bool ip_address_resolved() const
                {
                    return addresses_.empty();
                }

                bool name_specified() const
                {
                    return !name_.empty();
                }

              private:
                void resolve_ip()
                {
                }

                void get_index_from_name()
                {
#    ifdef _WIN32

#    else
                    index_ = ::if_nametoindex(name_.c_str());
#    endif
                }

                void fill_name_from_index()
                {
                    char buf[IF_NAMESIZE];
                    ::if_indextoname(index_, buf);
                    name_.assign(buf);
                }

                void fill_addr_from_ifname(const char* ifname,
                                           std::vector<ip::address>& addresses)
                {
                    ::ifaddrs* addrs;
                    ::getifaddrs(&addrs);
                    fill_addr_from_ifname_recurse(addrs, ifname, addresses);
                }

                void fill_addr_from_ifname_recurse(
                    ifaddrs* current, const char* ifname,
                    std::vector<ip::address>& addresses)
                {
                    if (current == NULL)
                        return;

                    if (::strcmp(current->ifa_name, ifname) == 0) {
                        if (current->ifa_addr->sa_family == AF_INET6) {

                            ip::address_v6::bytes_type bytes;
                            ip::address_v6::bytes_type::value_type *bbegin,
                                *bend;

                            bbegin = reinterpret_cast<
                                ip::address_v6::bytes_type::value_type*>(
                                &reinterpret_cast<sockaddr_in6*>(
                                     current->ifa_addr)
                                     ->sin6_addr);
                            bend = bbegin + sizeof(struct ::in6_addr);

                            std::copy(bbegin, bend, bytes.begin());
                            ip::address_v6 newaddr(bytes);

                            if (std::find(
                                    addresses.begin(), addresses.end(), newaddr)
                                == addresses.end())
                                addresses.push_back(newaddr);
                        }
                        else if (current->ifa_addr->sa_family == AF_INET) {

                            ip::address_v4 newaddr(
                                ntohl(reinterpret_cast<sockaddr_in*>(
                                          current->ifa_addr)
                                          ->sin_addr.s_addr));

                            if (std::find(
                                    addresses.begin(), addresses.end(), newaddr)
                                == addresses.end())
                                addresses.push_back(newaddr);
                        }
                    }

                    return fill_addr_from_ifname_recurse(
                        current->ifa_next, ifname, addresses);
                }

                std::string find_name_by_addr(ifaddrs* current,
                                              const ip::address& addr)
                {
                    if (addr.is_v4())
                        return find_name_by_addr_recurse_v4(
                            current, addr.to_v4().to_bytes());
                    else if (addr.is_v6())
                        return find_name_by_addr_recurse_v6(
                            current, addr.to_v6().to_bytes());

                    return {};
                }

                std::string find_name_by_addr_recurse_v6(
                    ifaddrs* current,
                    const ip::address_v6::bytes_type& addr_bytes)
                {
                    if (current == NULL)
                        return {};

                    if (current->ifa_addr->sa_family != AF_INET6)
                        return find_name_by_addr_recurse_v6(
                            current->ifa_next, addr_bytes);

                    if (memcmp(&reinterpret_cast<const sockaddr_in6*>(
                                    current->ifa_addr)
                                    ->sin6_addr,
                               reinterpret_cast<const in6_addr*>(
                                   addr_bytes.data()),
                               16)
                        == 0)
                        return std::string(current->ifa_name);

                    return find_name_by_addr_recurse_v6(
                        current->ifa_next, addr_bytes);
                }

                std::string find_name_by_addr_recurse_v4(
                    ifaddrs* current, const ip::address_v4::bytes_type& addr)
                {
                    if (current == NULL)
                        return {};

                    if (current->ifa_addr->sa_family != AF_INET)
                        return find_name_by_addr_recurse_v4(
                            current->ifa_next, addr);

                    if (memcmp(
                            &reinterpret_cast<sockaddr_in*>(current->ifa_addr)
                                 ->sin_addr,
                            addr.data(), 4)
                        == 0)
                        return std::string(current->ifa_name);

                    return find_name_by_addr_recurse_v4(
                        current->ifa_next, addr);
                }

                index_type index_ = 0;
                std::string name_;
                std::vector<ip::address> addresses_;
            };
        }    // namespace mdns
    }        // namespace asio
}    // namespace boost
#endif
