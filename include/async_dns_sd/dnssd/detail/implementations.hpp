#pragma once
#include "../config.hpp"
#include "concepts.hpp"

DNSSD_NAMESPACE_BEGIN
namespace detail {

namespace impls {
struct bonjour {
};
struct avahi {
};
}

#ifdef DNSSD_CONCEPTS
template <typename Implementation>
concept DNSSDImplementation
    = std::is_same_v<Implementation,
                     impls::bonjour> || std::is_same_v<Implementation, impls::avahi>;
#endif


}
DNSSD_NAMESPACE_END