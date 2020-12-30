#include "../../example/custom_alloc.hpp"
#include "dnssd.hpp"
#include <iostream>

// this file is only here to force Xcode to compile the headers
// and testing the whole thing during development

class browser: public std::enable_shared_from_this<browser> {
  public:
    explicit browser(boost::asio::io_context& io)
    : client_(io)
    , timer_(io)
    {
        std::cout << boost::asio::dnssd::client::implementation::name << "\n";
    }

    void run() {
        auto self = this->shared_from_this();

        client_.async_connect([self](const boost::system::error_code& ec){
            if (ec)
                std::cerr << "connection failed: " << ec.message() << "\n";
            else {
                if (self->client_.is_connected())
                    std::cout << "is connected to daemon " << self->client_.daemon_version() << "\n";

                self->timer_.expires_after(std::chrono::seconds(5));
                self->timer_.async_wait([self](boost::system::error_code ec){
                    std::cerr << "wait: " << ec.message() << "\n";
                    // now we go out of scope and everything is destroyed
                });
            }
        });
    }

  private:
    boost::asio::dnssd::client client_;
    boost::asio::steady_timer timer_;
};

int main()
{
    boost::asio::io_context ctx;

    std::make_shared<browser>(ctx)->run();

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
