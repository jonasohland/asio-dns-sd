#include <async_dns_sd/dnssd.hpp>


int main()
{
    boost::asio::io_context ioc;
    boost::asio::dnssd::tcp_browser browser { ioc };
    boost::asio::dnssd::service_record record;
    boost::system::error_code ec;
    
    
    browser.open(boost::asio::dnssd::network_interface::all(),
                 boost::asio::dnssd::tcp_service::type("_http"), "local.");
    
    while (!ec) {
        bool add = browser.browse(record, ec);
        std::cout << (add? "Add " : "Remove ") << record.name() << "\n";
    }
    
    browser.close();
    return ec.value();
}
