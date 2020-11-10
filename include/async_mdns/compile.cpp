#include "mdns.hpp"
#include <iostream>

// this file is only here to force XCode to compile the headers
// and testing the whole thing during development

class nbrowser: public std::enable_shared_from_this<nbrowser> {
  public:
    nbrowser(boost::asio::io_context& ctx)
        : browser_(ctx)
    {
    }

    void run()
    {
        boost::system::error_code ec;
        
        // open browser for http services on all interfaces
        browser_.open(boost::asio::mdns::network_interface::all(),
                      "_http._tcp", "local", ec);

        if (ec) {
            std::cerr << ec.message() << "\n";
            return;
        }

        // start browsing
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

        // do more browsing
        do_browse();
    }

    boost::asio::mdns::browser browser_;
};

int main()
{
    boost::asio::io_context ctx;
    
    // start browser
    std::make_shared<nbrowser>(ctx)->run();
    
    // run context in this thread
    ctx.run();
    
    // bye
    return 0;
}
