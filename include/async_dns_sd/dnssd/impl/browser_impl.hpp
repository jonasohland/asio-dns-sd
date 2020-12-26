#pragma once

#include "browser_impl_bonjour.hpp"

namespace boost {
namespace asio {
namespace dnssd {
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
}    // namespace dnssd
}    // namespace asio
}    // namespace boost
