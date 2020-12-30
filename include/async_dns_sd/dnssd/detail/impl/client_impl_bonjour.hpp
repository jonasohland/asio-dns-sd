#pragma once
#include "../../waitable_descriptor.hpp"
#include "../helpers.hpp"
#include "async_dns_sd/dnssd/config.hpp"
#include <boost/asio/executor.hpp>
#include <boost/beast/core/async_base.hpp>
#include <dns_sd.h>

DNSSD_NAMESPACE_BEGIN

namespace impl {

template <typename Executor = DNSSD_NET_LIB::executor>
class client_impl_bonjour {
  public:
    DNSSD_NO_COPY(client_impl_bonjour)

    using wait_type = typename waitable_descriptor<Executor>::wait_type;
    static constexpr const char* name = "Apple Bonjour";

    template <typename ExecutionContext>
    explicit client_impl_bonjour(
        ExecutionContext& context,
        typename enable_if<is_convertible<
            ExecutionContext&, execution_context&>::value>::type* = 0)
        : desc_(context)
    {
    }

    template <typename Handler>
    struct async_connect_op: boost::beast::async_base<Handler, Executor> {

        DNSSD_NO_COPY(async_connect_op);

        explicit async_connect_op(Handler&& handler,
                                  client_impl_bonjour<Executor>* owner)
            : boost::beast::async_base<Handler, Executor>(
                std::forward<Handler>(handler), owner->desc_.get_executor())
            , owner_(owner)
        {
            (*this)({}, false);
        }

        void operator()(boost::system::error_code ec, bool = true)
        {
            uint32_t ver, size = sizeof(uint32_t);

            if ((ec = make_dnssd_error(DNSServiceCreateConnection(&owner_->service_))))
                this->complete(false, ec);

            owner_->desc_.assign(DNSServiceRefSockFD(owner_->service_), ec);
            owner_->desc_.async_wait(
                waitable_descriptor<Executor>::wait_read, await_err_op(owner_));

            if (ec)
                this->complete(false, ec);

            if ((ec = make_dnssd_error(DNSServiceGetProperty(
                     kDNSServiceProperty_DaemonVersion, &ver, &size))))
                return this->complete(false, ec);

            std::stringstream str;
            str << ver / 10000 << "." << ver / 100 % 100;
            owner_->daemon_version_.assign(str.str());

            this->complete(false, ec);
        }

        client_impl_bonjour* owner_;
    };

    struct await_err_op {
        DNSSD_NO_COPY(await_err_op);

        explicit await_err_op(client_impl_bonjour<Executor>* owner)
            : owner_(owner)
            , work_(owner->desc_.get_executor())
        {
            owner_->await_err_op_ = this;
        }

        await_err_op(await_err_op&& other)
            : owner_(other.owner_)
            , work_(std::move(other.work_))
        {
            owner_->await_err_op_ = this;
        }

        void operator()(const boost::system::error_code& err)
        {
            std::cout << "await_err_op: " << err.message() << "\n";
            owner_->is_open_ = false;
            work_.reset();
        }

        boost::asio::executor_work_guard<Executor> work_;
        client_impl_bonjour<Executor>* owner_;
    };

    template <typename CompletionToken>
    BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken,
                                  void(const boost::system::error_code& ec))
    async_connect(CompletionToken&& token)
    {
        using completion_type = DNSSD_NET_LIB::async_completion<
            CompletionToken, void(const boost::system::error_code&)>;

        completion_type completion { token };

        async_connect_op<typename completion_type::completion_handler_type> op {
            std::move(completion.completion_handler), this
        };

        return completion.result.get();
    }

    std::string daemon_version() const
    {
        return daemon_version_;
    }

    bool is_connected() const
    {
        return desc_.is_open();
    }

  private:
    DNSServiceRef service_;
    waitable_descriptor<Executor> desc_;
    await_err_op* await_err_op_;
    bool is_open_ = false;
    std::string daemon_version_;
};
}    // namespace impl

DNSSD_NAMESPACE_END