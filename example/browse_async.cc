#include "custom_alloc.hpp"
#include <async_dns_sd/dnssd.hpp>

// this file is only here to force Xcode to compile the headers
// and testing the whole thing during development

class nbrowser: public std::enable_shared_from_this<nbrowser> {
  public:
    nbrowser(boost::asio::io_context& ctx)
        : record_(TestAlloc<char>())
        , browser_(ctx)
        , resolver_(ctx)
    {
    }

    void run()
    {
        boost::system::error_code ec;

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
            record_, [self](boost::system::error_code ec, bool add) {
                if (ec) {
                    std::cout << ec.message() << "\n";
                    return;
                }
                std::cout << (add ? "Add " : "Remove ")
                          << "service: " << self->record_.name() << "\n";
                self->do_browse();
            });
    }

    boost::asio::dnssd::basic_service_record<TestAlloc<char>> record_;
    boost::asio::dnssd::tcp_browser browser_;
    boost::asio::ip::tcp::resolver resolver_;
};

int main()
{
    boost::asio::io_context ctx;

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
