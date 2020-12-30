#pragma once

#pragma once

#include "basic_service.hpp"
#include "error.hpp"
#include "detail/impl/impl.hpp"
#include "network_interface.hpp"
#include "service_record.hpp"
#include "util.hpp"

#include <boost/asio.hpp>
#include <boost/asio/posix/basic_stream_descriptor.hpp>
#include <iostream>

namespace boost {
namespace asio {
namespace dnssd {

template <typename Transport, typename Executor = executor,
          typename Implementation
          = impl::default_implementations::browser<Transport, Executor>>
class basic_browser {
  public:
    using executor_type = Executor;
    using transport     = Transport;

    explicit basic_browser(const Executor& exec)
        : impl_(exec)
    {
    }

    template <typename ExecutionContext>
    explicit basic_browser(
        ExecutionContext& context,
        typename enable_if<is_convertible<
            ExecutionContext&, execution_context&>::value>::type* = 0)
        : impl_(context)
    {
    }

    void open(const network_interface& interface,
              const typename basic_service<Transport>::type& type,
              const std::string& domain)
    {
        impl_.open(interface, type, domain);
    }

    void open(const network_interface& interface,
              const typename basic_service<Transport>::type& type,
              const std::string& domain,
              boost::system::error_code& errc) BOOST_NOEXCEPT
    {
        impl_.open(interface, type, domain, errc);
    }

    void close()
    {
        impl_.close();
    }

    void close(system::error_code& err)
    {
        impl_.close(err);
    }

    bool is_open() const noexcept
    {
        return impl_.is_open();
    }

    // TODO: Turn this into an asio-style composed operation
    template <typename CompletionToken, typename Allocator>
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken,
                                  void(boost::system::error_code, bool))
    async_browse(basic_service_record<Allocator>& record,
                 CompletionToken&& token)
    {
        return impl_.async_browse(record, std::forward<CompletionToken>(token));
    }

    // TODO: Turn this into an asio-style composed operation
    template <typename CompletionToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken,
                                  void(boost::system::error_code, bool))
    async_browse(CompletionToken&& token)
    {
        return impl_.async_browse(std::forward<CompletionToken>(token));
    }

    template <typename RecordType>
    bool browse(RecordType& record)
    {
        return impl_.browse(record);
    }

    template <typename RecordType>
    bool browse(RecordType& record, system::error_code& ec)
    {
        return impl_.browse(record, ec);
    }

    executor_type get_executor() BOOST_ASIO_NOEXCEPT
    {
        return impl_.get_executor();
    }

  private:
    Implementation impl_;
};

using tcp_browser = basic_browser<boost::asio::ip::tcp>;
using udp_browser = basic_browser<boost::asio::ip::udp>;
}    // namespace dnssd
}    // namespace asio
}    // namespace boost
