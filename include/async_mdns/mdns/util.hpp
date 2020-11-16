#pragma once

#include <boost/asio.hpp>
#include <dns_sd.h>
#include <ifaddrs.h>

namespace boost {
    namespace asio {
        namespace mdns {
            namespace detail {

                int ip_to_ifname(const ip::address& addr, std::string& ifname)
                {
                    auto v4                = addr.to_v4();
                    unsigned long local_if = ntohl(v4.to_uint());

                    if (!local_if)
                        return -1;

                    ifaddrs *addrs, *firstaddr, *found;
                    getifaddrs(&addrs);
                    firstaddr = addrs;
                    found     = nullptr;

                    while (addrs != NULL) {
                        if (local_if
                            == reinterpret_cast<unsigned int>(
                                reinterpret_cast<sockaddr_in*>(addrs->ifa_addr)
                                    ->sin_addr.s_addr)) {
                            found = addrs;
                            break;
                        }
                        addrs = addrs->ifa_next;
                    }

                    freeifaddrs(firstaddr);

                    if (found) {
                        ifname.assign(found->ifa_name);
                        return 0;
                    }
                    else {
                        return 1;
                    }
                }

                unsigned int if_index_from_ifname(const std::string& ifname)
                {
                    return if_nametoindex(ifname.c_str());
                }

                unsigned int if_index_from_ip_addr(const ip::address& addr)
                {
                    std::string ifname;
                    int err = ip_to_ifname(addr, ifname);

                    if (err == -1)
                        return -1;

                    return if_index_from_ifname(ifname);
                }

                int ip6_compare(const struct in6_addr* lhs,
                                const struct in6_addr* rhs)
                {
                    return memcmp(lhs, rhs, 16);
                }

            }    // namespace detail
        }        // namespace mdns
    }            // namespace asio
}    // namespace boost
