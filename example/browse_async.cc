#include "custom_alloc.hpp"
#include <async_dns_sd/dnssd.hpp>
#include <deque>
#include <forward_list>

// this file is only here to force Xcode to compile the headers
// and testing the whole thing during development

class nbrowser: public std::enable_shared_from_this<nbrowser> {
  public:
    nbrowser(boost::asio::io_context& ctx)
        : record_(TestAlloc<char>())
        , browser_(ctx)
        , timer_(ctx)
        , resolver_(ctx)
    {
    }

    void run()
    {
        boost::system::error_code ec;

        client_.open();

        // open browser for http services on all interfaces
        browser_.open(boost::asio::dnssd::network_interface::all(),
                      boost::asio::dnssd::tcp_service::type("_http"), "local",
                      ec);

        if (ec) {
            std::cerr << ec.message() << "\n";
            return;
        }

        // start browsing
        do_browse();
    }

    void do_browse()
    {
        auto self = this->shared_from_this();
        browser_.async_browse(
            record_, [self](const boost::system::error_code& ec, bool add) {
                if (ec) {
                    if (ec.value()
                        == static_cast<int>(boost::asio::dnssd::service_error::
                                                service_not_running)) {
                        return self->restart(self);
                    }
                    std::cerr << ec.message() << "\n";
                    return;
                }
                std::cout << (add ? "Add " : "Remove ")
                          << "service: " << self->record_.name() << "\n";
                self->do_browse();
            });
    }

    void restart(const std::shared_ptr<nbrowser>& self)
    {
        browser_.close();

        std::cout << "Try reconnecting to daemon in 5 seconds\n";
        timer_.expires_from_now(std::chrono::seconds(5));
        timer_.async_wait([self](const boost::system::error_code& err) {
            if (err) {
                std::cerr << "Failed to restart browse operation: "
                          << err.message() << "\n";
                return;
            }
            self->run();
        });
    }

    boost::asio::dnssd::basic_service_record<TestAlloc<char>> record_;
    boost::asio::dnssd::tcp_browser browser_;
    boost::asio::dnssd::client_reference client_;
    boost::asio::steady_timer timer_;
    boost::asio::ip::tcp::resolver resolver_;
};

int main()
{
    boost::asio::io_context ctx;
    
    std::cout << sizeof(std::forward_list<boost::asio::dnssd::service_record>) << "\n";

    // start browser
    std::make_shared<nbrowser>(ctx)->run();

    for (;;) {
        try {
            ctx.run();
            break;
        }
        catch (std::exception& e) {
            std::cerr << "Oh no: " << e.what() << "\n";
        }
    }

    // bye
    return 0;
}
