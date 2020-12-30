#pragma once


#ifndef DNSSD_DONT_USE_BOOST_ASIO_NAMESPACE
#    define DNSSD_NAMESPACE boost::asio::dnssd
#    ifdef __cpp_nested_namespace_definitions
#        define DNSSD_NAMESPACE_BEGIN namespace boost::asio::dnssd {
#    else
#        define DNSSD_NAMESPACE_BEGIN                                          \
            namespace boost {                                                  \
            namespace asio {                                                   \
            namespace dnssd {
#    endif
#    ifdef __cpp_nested_namespace_definitions
#        define DNSSD_NAMESPACE_END }
#    else
#        define DNSSD_NAMESPACE_END                                            \
            }                                                                  \
            }                                                                  \
            }
#    endif
#    define DNSSD_NAMESPACE_ROOT boost::asio::dnssd
#elif
#    define DNSSD_NAMESPACE       dnssd
#    define DNSSD_NAMESPACE_BEGIN namespace dnssd {
#    define DNSSD_NAMESPACE_END   }
#endif
#define DNSSD_NET_LIB ::boost::asio

#define DNSSD_USE_BONJOUR_IMPL