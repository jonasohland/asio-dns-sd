#pragma once

#include "async_dns_sd/dnssd/config.hpp"
#include "browser_impl_bonjour.hpp"
#include "client_impl_bonjour.hpp"

DNSSD_NAMESPACE_BEGIN

namespace impl {

namespace implementations {
struct bonjour {
};

struct avahi {
};

struct standalone {
};
}    // namespace implementations

template <typename ImplementationType>
struct default_implementation_set {
};

template <>
struct default_implementation_set<implementations::bonjour> {
    template <typename Transport, typename Executor>
    using browser = browser_impl_bonjour<Transport, Executor>;

    template <typename Executor>
    using client = client_impl_bonjour<Executor>;

};

template <>
struct default_implementation_set<implementations::avahi> {
};

template <>
struct default_implementation_set<implementations::standalone> {
};

using default_implementations
    = default_implementation_set<implementations::bonjour>;

}    // namespace impl

DNSSD_NAMESPACE_END