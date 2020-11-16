#pragma once

#pragma once

#include "error.hpp"
#include "network_interface.hpp"
#include "service_record.hpp"
#include "util.hpp"

#include <boost/asio.hpp>
#include <boost/asio/posix/basic_stream_descriptor.hpp>
#include <dns_sd.h>
#include <ifaddrs.h>
#include <iostream>

namespace boost {
    namespace asio {
        namespace mdns {

            template <typename Executor = executor>
            class basic_browser {
              public:
                using executor_type = Executor;

                explicit basic_browser(const Executor& exec)
                    : stream(exec)
                {
                }


                template <typename ExecutionContext>
                explicit basic_browser(
                    ExecutionContext& context,
                    typename enable_if<
                        is_convertible<ExecutionContext&,
                                       execution_context&>::value>::type* = 0)
                    : stream(context)
                {
                }

                ~basic_browser()
                {
                    DNSServiceRefDeallocate(ref);
                }

                void open(network_interface&& interface,
                          const std::string& type, const std::string& domain)
                {
                    DNSServiceErrorType err
                        = open_impl(interface.index(), type, domain);
                    boost::asio::detail::throw_error(make_mdns_error(err));
                    stream.assign(DNSServiceRefSockFD(ref));
                }

                void open(network_interface&& interface,
                          const std::string& type, const std::string& domain,
                          boost::system::error_code& errc) BOOST_NOEXCEPT
                {
                    DNSServiceErrorType err
                        = open_impl(interface.index(), type, domain);
                    errc.assign(err, get_dns_service_error_category());

                    if (errc)
                        return;

                    stream.assign(DNSServiceRefSockFD(ref), errc);
                }

                template <typename CompletionHandler>
                void async_browse(CompletionHandler&& handler)
                {
                    last_records_.clear();
                    stream.async_wait(
                        posix::basic_stream_descriptor<Executor>::wait_read,
                        [handler, this](boost::system::error_code ec) {
                            if (ec)
                                handler(ec);
                            else
                                handler(make_mdns_error(
                                    DNSServiceProcessResult(ref)));
                        });
                }

                executor_type get_executor() BOOST_ASIO_NOEXCEPT
                {
                    return stream.get_executor();
                }

                std::vector<service_record> const& records() const
                {
                    return last_records_;
                }

              private:
                void dns_browse_reply()
                {
                }

                DNSServiceErrorType open_impl(unsigned int if_idx,
                                              const std::string& type,
                                              const std::string& domain)
                {
                    return DNSServiceBrowse(
                        &ref, 0, if_idx, type.c_str(), NULL,
                        [](DNSServiceRef sdRef, DNSServiceFlags flags,
                           uint32_t interfaceIndex,
                           DNSServiceErrorType errorCode,
                           const char* serviceName, const char* regtype,
                           const char* replyDomain, void* context) {
                            auto self
                                = reinterpret_cast<basic_browser<Executor>*>(
                                    context);
                            self->last_records_.emplace_back(
                                interfaceIndex, serviceName, regtype,
                                replyDomain);
                        },
                        (void*) this);
                }

                std::vector<service_record> last_records_;
                DNSServiceRef ref;
                posix::basic_stream_descriptor<Executor> stream;
            };

            using browser = basic_browser<>;
        }    // namespace mdns
    }        // namespace asio
}    // namespace boost
