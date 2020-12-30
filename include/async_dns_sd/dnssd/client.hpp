#pragma once
#include "detail/impl/client_impl_bonjour.hpp"
#include "detail/implementations.hpp"
#include <boost/asio/ts/executor.hpp>

DNSSD_NAMESPACE_BEGIN

#ifdef __cpp_nested_namespace_definitions
namespace detail::impl {
#else
namespace detail {
namespace impl {
#endif

template <DNSSD_CONCEPT(detail::DNSSDImplementation) ImplementationType,
          typename Executor>
struct select_client_impl {
};

template <typename Executor>
struct select_client_impl<detail::impls::bonjour, Executor> {
    using implementation_type = detail::impls::bonjour;
    using implementation =
        typename DNSSD_NAMESPACE_ROOT::impl::client_impl_bonjour<Executor>;
};

template <typename Executor>
struct select_client_impl<detail::impls::avahi, Executor> {
    using implementation_type = detail::impls::avahi;
    using implementation =
        typename DNSSD_NAMESPACE_ROOT::impl::client_impl_bonjour<Executor>;
};

#ifdef __cpp_nested_namespace_definitions
}
#else
}
}
#endif

/**
 * A basic client implementation
 */
template <DNSSD_CONCEPT(detail::DNSSDImplementation) ImplementationType,
          typename Executor = DNSSD_NET_LIB::executor>
struct basic_client {
  public:
    using implementation =
        typename detail::impl::select_client_impl<ImplementationType,
                                                  Executor>::implementation;
    using wait_type =
        typename implementation::wait_type;

    template <typename ExecutionContext>
    explicit basic_client(
        ExecutionContext& context,
        typename enable_if<is_convertible<
            ExecutionContext&, execution_context&>::value>::type* = 0)
        : impl_(context)
    {
    }

    std::string daemon_version()
    {
        return impl_.daemon_version();
    }

    [[nodiscard]] constexpr const char* implementation_name() const noexcept
    {
        return implementation::name;
    }

    bool is_connected() const
    {
        return impl_.is_connected();
    }

    template <typename CompletionToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken,
                                  void(const boost::system::error_code& ec))
    async_connect(CompletionToken&& token)
    {
        return impl_.async_connect(std::forward<CompletionToken>(token));
    }

  private:
    implementation impl_;
};

template <typename Executor = DNSSD_NET_LIB::executor>
using bonjour_client = basic_client<detail::impls::bonjour, Executor>;

template <typename Executor = DNSSD_NET_LIB::executor>
using avahi_client = basic_client<detail::impls::avahi, Executor>;

template <typename Executor = DNSSD_NET_LIB::executor>
#ifdef DNSSD_USE_BONJOUR_IMPL
using default_client = bonjour_client<Executor>;
#elif DNSSD_USE_AVAHI_IMPL
using default_client = avahi_client<Executor>;
#endif

using client = default_client<>;

DNSSD_NAMESPACE_END