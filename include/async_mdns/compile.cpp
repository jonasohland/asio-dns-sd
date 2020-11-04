#include "mdns.hpp"
#include <iostream>

class nbrowser: public std::enable_shared_from_this<nbrowser> {
  public:
    nbrowser(boost::asio::io_context& ctx)
        : browser_(ctx)
    {
    }

    void run()
    {
        boost::system::error_code ec;
        browser_.open(boost::asio::ip::address::from_string("0.0.0.0"),
                      "_http._tcp", "local", ec);

        if (ec) {
            std::cerr << ec.message() << "\n";
            return;
        }

        do_browse();
    }

    void do_browse()
    {
        browser_.async_browse(std::bind(&nbrowser::on_browse_complete,
                                        this->shared_from_this(),
                                        std::placeholders::_1));
    }

    void on_browse_complete(boost::system::error_code ec)
    {
        if (ec) {
            std::cout << "Browse error: " << ec.message() << "\n";
            return;
        }

        for (const auto& record : browser_.records()) {
            std::cout << "Found service: " << record.fqdn()
                      << " interface: " << record.interface().name() << "\n";
        }

        do_browse();
    }

    boost::asio::mdns::browser browser_;
};

int main()
{
    boost::asio::io_context ctx;
    std::make_shared<nbrowser>(ctx)->run();
    ctx.run();
    return 0;
}
