#pragma once

#include "../service_record.hpp"
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/beast/core/async_base.hpp>
#include <boost/core/ignore_unused.hpp>
#include <dns_sd.h>

namespace boost {
namespace asio {
namespace dnssd {
namespace impl {

template <typename Transport, typename Executor>
class browser_impl_bonjour {
  public:
    using transport_type = Transport;
    using executor_type  = Executor;

    explicit browser_impl_bonjour(const Executor& exec)
        : filed_(exec)
    {
    }


    template <typename ExecutionContext>
    explicit browser_impl_bonjour(
        ExecutionContext& context,
        typename enable_if<is_convertible<
            ExecutionContext&, execution_context&>::value>::type* = 0)
        : filed_(context)
    {
    }

    void open(const network_interface& interface,
              const typename basic_service<Transport>::type& type,
              const std::string& domain)
    {
        DNSServiceErrorType err
            = do_open(interface.index(), type.service_type_string(), domain);
        boost::asio::detail::throw_error(make_dnssd_error(err));
        filed_.assign(DNSServiceRefSockFD(sref_));
    }

    void open(const network_interface& interface,
              const typename basic_service<Transport>::type& type,
              const std::string& domain,
              boost::system::error_code& errc) BOOST_NOEXCEPT
    {
        DNSServiceErrorType err
            = do_open(interface.index(), type.service_type_string(), domain);
        errc.assign(err, get_dns_service_error_category());

        if (errc)
            return;

        filed_.assign(DNSServiceRefSockFD(sref_), errc);
    }

    void close()
    {
        if (sref_)
            DNSServiceRefDeallocate(sref_);
        filed_.close();
    }

    void close(boost::system::error_code& ec)
    {
        if (sref_)
            DNSServiceRefDeallocate(sref_);
        filed_.close(ec);
    }

    struct async_browse_op_base {
        virtual void write(const char*&, const char*&, const char*&,
                           network_interface::index_type)
            = 0;


        void fail(DNSServiceErrorType err)
        {
        }

        void set_flags(DNSServiceFlags flags)
        {
        }
    };

    template <typename Handler, typename Allocator = service_record::allocator>
    struct async_browse_operation
        : async_browse_op_base
        , beast::async_base<Handler, executor_type>
        , coroutine {

        async_browse_operation(async_browse_operation&& other)
            : beast::async_base<Handler, executor_type>(
                std::forward<beast::async_base<Handler, executor_type>>(other))
            , sref_(other.sref_)
            , op_ctx_(other.op_ctx_)
            , rec_(other.rec_)
            , stream_(other.stream_)
        {
            // write our new context location for DNSServiceBrowseReply callback
            *op_ctx_ = this;
        }

        async_browse_operation()                              = delete;
        async_browse_operation(const async_browse_operation&) = delete;
        void operator=(const async_browse_operation&) = delete;
        void operator=(async_browse_operation&&) = delete;

        async_browse_operation(
            Handler&& handler,
            posix::basic_stream_descriptor<executor_type>& stream,
            DNSServiceRef sref, async_browse_op_base** op_ctx)
            : beast::async_base<Handler, executor_type>(
                std::forward<Handler>(handler), stream.get_executor())
            , sref_(sref)
            , op_ctx_(op_ctx)
            , stream_(stream)
        {
            *op_ctx = this;
            (*this)({}, false);
        }

        async_browse_operation(
            basic_service_record<Allocator>* record, Handler&& handler,
            posix::basic_stream_descriptor<executor_type>& stream,
            DNSServiceRef sref, async_browse_op_base** op_ctx)
            : beast::async_base<Handler, executor_type>(
                std::forward<Handler>(handler), stream.get_executor())
            , sref_(sref)
            , op_ctx_(op_ctx)
            , rec_(record)
            , stream_(stream)
        {
            *op_ctx = this;
            (*this)({}, false);
        }

        void operator()(const system::error_code& err,
                        bool is_continuation = true)
        {
            if (is_continuation) {
                ::DNSServiceProcessResult(sref_);
                this->complete(false, system::error_code());
                return;
            }
            else {
                stream_.async_wait(
                    posix::basic_stream_descriptor<executor_type>::wait_read,
                    std::move(*this));
            }
        }

        void write(const char*& name, const char*& type, const char*& domain,
                   network_interface::index_type if_idx) final
        {
            rec_->name(name);
            rec_->type(type);
            rec_->domain(domain);
            rec_->if_index(if_idx);
        }

        DNSServiceRef sref_;
        async_browse_op_base** op_ctx_;
        basic_service_record<Allocator>* rec_;
        posix::basic_stream_descriptor<executor_type>& stream_;
    };

    template <typename CompletionToken, typename Allocator>
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken,
                                  void(boost::system::error_code))
    async_browse(basic_service_record<Allocator>& record,
                 CompletionToken&& token)
    {
        async_completion<CompletionToken, void(system::error_code)> completion(
            token);
        async_browse_operation<CompletionToken, Allocator> op(
            &record, std::move(completion.completion_handler), filed_, sref_,
            &current_op_);
        return completion.result.get();
    }

    template <typename CompletionToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken,
                                  void(boost::system::error_code,
                                       const service_record&))
    async_browse(CompletionToken&& token)
    {
        async_completion<CompletionToken, void(system::error_code)> completion(
            token);
        async_browse_operation<CompletionToken> op(
            std::forward<CompletionToken>(token), filed_, sref_, &current_op_);
        return completion.result.get();
    }

    ~browser_impl_bonjour()
    {
        if (sref_)
            DNSServiceRefDeallocate(sref_);
    }

    executor_type get_executor() BOOST_ASIO_NOEXCEPT
    {
        return filed_.get_executor();
    }

  private:
    DNSServiceErrorType do_open(unsigned int if_idx, const std::string& type,
                                const std::string& domain) noexcept
    {
        return DNSServiceBrowse(
            &sref_, 0, if_idx, type.c_str(), domain.c_str(),
            &browser_impl_bonjour<Transport, Executor>::dns_service_browse_cb,
            &current_op_);
    }

    static void dns_service_browse_cb(
        DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex,
        DNSServiceErrorType errorCode, const char* serviceName,
        const char* regtype, const char* replyDomain, void* context)
    {
        ignore_unused(sdRef);
        auto op_ctx = *reinterpret_cast<async_browse_op_base**>(context);

        if (errorCode != kDNSServiceErr_NoError)
            return op_ctx->fail(errorCode);

        op_ctx->set_flags(flags);
        op_ctx->write(serviceName, regtype, replyDomain, interfaceIndex);
    }

    DNSServiceRef sref_ = nullptr;
    async_browse_op_base* current_op_;
    posix::basic_stream_descriptor<Executor> filed_;
};

}    // namespace impl
}    // namespace dnssd
}    // namespace asio
}    // namespace boost