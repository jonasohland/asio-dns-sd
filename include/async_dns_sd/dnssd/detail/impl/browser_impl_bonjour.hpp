#pragma once

#include "async_dns_sd/dnssd/service_record.hpp"
#include "async_dns_sd/dnssd/waitable_descriptor.hpp"
#include <boost/asio/posix/stream_descriptor.hpp>
#include <boost/beast/core/async_base.hpp>
#include <boost/core/ignore_unused.hpp>
#include <dns_sd.h>
#include <iostream>

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
        ::boost::asio::detail::throw_error(make_dnssd_error(err));
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
        sref_ = nullptr;
        filed_.release();
    }

    void close(boost::system::error_code& ec)
    {
        if (sref_)
            DNSServiceRefDeallocate(sref_);
        sref_ = nullptr;
        filed_.release();
    }

    bool is_open() const
    {
        return sref_ && filed_.is_open();
    }

    struct browse_op_base {
        virtual void write(const char*&, const char*&, const char*&,
                           network_interface::index_type)
            = 0;

        void fail(DNSServiceErrorType err)
        {
            err_ = err;
        }

        void set_flags(DNSServiceFlags flags)
        {
            flags_ = flags;
        }

        bool is_record_add() const noexcept
        {
            return flags_ & kDNSServiceFlagsAdd;
        }
        
        bool more_coming() const noexcept
        {
            return flags_ & kDNSServiceFlagsMoreComing;
        }

        system::error_code get_error() const
        {
            return make_dnssd_error(err_);
        }

        DNSServiceErrorType err_ = kDNSServiceErr_NoError;
        DNSServiceFlags flags_   = 0;
    };

    template <typename Handler, typename Allocator = service_record::allocator>
    struct async_browse_operation
        : browse_op_base
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

        async_browse_operation(Handler&& handler,
                               waitable_descriptor<executor_type>& stream,
                               DNSServiceRef sref, browse_op_base** op_ctx)
            : beast::async_base<Handler, executor_type>(
                std::forward<Handler>(handler), stream.get_executor())
            , sref_(sref)
            , op_ctx_(op_ctx)
            , stream_(stream)
        {
            *op_ctx = this;
            if (stream_.still_readable())
                continue_reading();
            else
                (*this)({}, false);
        }

        async_browse_operation(basic_service_record<Allocator>* record,
                               Handler&& handler,
                               waitable_descriptor<executor_type>& stream,
                               DNSServiceRef sref, browse_op_base** op_ctx)
            : beast::async_base<Handler, executor_type>(
                std::forward<Handler>(handler), stream.get_executor())
            , sref_(sref)
            , op_ctx_(op_ctx)
            , rec_(record)
            , stream_(stream)
        {
            *op_ctx = this;
            if (stream_.still_readable())
                continue_reading();
            else
                (*this)({}, false);
        }

        void operator()(const system::error_code& err,
                        bool is_continuation = true)
        {
            if (err)
                return this->complete(false, err, false);
            
            if (is_continuation) {
                if (auto err
                    = make_dnssd_error(::DNSServiceProcessResult(sref_)))
                    return this->complete(false, err, false);
                else {
                    if (this->more_coming())
                        stream_.still_readable(true);
                    this->complete(
                        false, this->get_error(), this->is_record_add());
                }
            }
            else
                stream_.async_wait(
                    waitable_descriptor<executor_type>::wait_read,
                    std::move(*this));
        }
            
        void continue_reading()
        {
            if (auto err = make_dnssd_error(::DNSServiceProcessResult(sref_)))
                return this->complete(false, err, false);
            else {
                if (!this->more_coming())
                    stream_.still_readable(false);
                return this->complete(false, err, false);
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
        browse_op_base** op_ctx_;
        basic_service_record<Allocator>* rec_;
        waitable_descriptor<executor_type>& stream_;
    };

    template <typename Allocator>
    struct sync_browse_operation: browse_op_base {

        sync_browse_operation(basic_service_record<Allocator>& rec)
            : rec_(rec)
        {
        }

        void write(const char*& name, const char*& type, const char*& domain,
                   network_interface::index_type if_idx) final
        {
            rec_.name(name);
            rec_.type(type);
            rec_.domain(domain);
            rec_.if_index(if_idx);
        }

        basic_service_record<Allocator>& rec_;
    };
    // this is stupid
    template <typename CompletionToken, typename Allocator>
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken,
                                  void(boost::system::error_code, bool))
    async_browse(basic_service_record<Allocator>& record,
                 CompletionToken&& token)
    {
        using completion_type
            = async_completion<CompletionToken, void(system::error_code, bool)>;

        completion_type completion(token);
        async_browse_operation<
            typename completion_type::completion_handler_type, Allocator>
            op(&record, std::move(completion.completion_handler), filed_, sref_,
               &op_ctx_);

        return completion.result.get();
    }

    template <typename CompletionToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken,
                                  void(boost::system::error_code, bool))
    async_browse(CompletionToken&& token)
    {
        using completion_type
            = async_completion<CompletionToken, void(system::error_code, bool)>;

        completion_type completion(token);
        async_browse_operation<
            typename completion_type::completion_handler_type>
            op(std::forward<CompletionToken>(token), filed_, sref_, &op_ctx_);

        return completion.result.get();
    }

    template <typename RecordType>
    bool browse(RecordType& record)
    {
        sync_browse_operation<typename RecordType::allocator> op(record);
        op_ctx_ = &op;

        filed_.wait(waitable_descriptor<executor_type>::wait_read);
        DNSServiceProcessResult(sref_);

        if (auto err = op.get_error())
            ::boost::asio::detail::throw_error(err);

        return op.record_add();
    }

    template <typename RecordType>
    bool browse(RecordType& record, system::error_code& err) noexcept
    {
        sync_browse_operation<typename RecordType::allocator> op(record);
        op_ctx_ = &op;

        filed_.wait(waitable_descriptor<executor_type>::wait_read, err);
        if (err)
            return false;
        DNSServiceProcessResult(sref_);

        err = op.get_error();
        return op.is_record_add();
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
            &op_ctx_);
    }

    static void dns_service_browse_cb(
        DNSServiceRef sdRef, DNSServiceFlags flags, uint32_t interfaceIndex,
        DNSServiceErrorType errorCode, const char* serviceName,
        const char* regtype, const char* replyDomain, void* context)
    {
        ignore_unused(sdRef);
        auto op_ctx = *reinterpret_cast<browse_op_base**>(context);

        if (errorCode != kDNSServiceErr_NoError)
            return op_ctx->fail(errorCode);
        
        std::cout << "more coming " << (flags & kDNSServiceFlagsMoreComing) << "\n";

        op_ctx->set_flags(flags);
        op_ctx->write(serviceName, regtype, replyDomain, interfaceIndex);
    }

    DNSServiceRef sref_ = nullptr;
    browse_op_base* op_ctx_;
    waitable_descriptor<Executor> filed_;
};

}    // namespace impl
}    // namespace dnssd
}    // namespace asio
}    // namespace boost
