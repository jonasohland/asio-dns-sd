#pragma once

#include "browser_impl_bonjour.hpp"

namespace boost {
namespace asio {
namespace dnssd {
namespace impl {
struct default_implementations {
    template <typename Transport, typename Executor>
    using browser = browser_impl_bonjour<Transport, Executor>;
};
}    // namespace impl
}    // namespace dnssd
}    // namespace asio
}    // namespace boost
