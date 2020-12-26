#include "dnssd.hpp"
#include <iostream>

// this file is only here to force Xcode to compile the headers
// and testing the whole thing during development

template <class T>
class MyAlloc {
  public:
    // type definitions
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    // rebind allocator to type U
    template <class U>
    struct rebind {
        typedef MyAlloc<U> other;
    };

    // return address of values
    pointer address(reference value) const
    {
        return &value;
    }
    const_pointer address(const_reference value) const
    {
        return &value;
    }

    /* constructors and destructor
     * - nothing to do because the allocator has no state
     */
    MyAlloc() throw()
    {
    }
    MyAlloc(const MyAlloc&) throw()
    {
    }
    template <class U>
    MyAlloc(const MyAlloc<U>&) throw()
    {
    }
    ~MyAlloc() throw()
    {
    }

    // return maximum number of elements that can be allocated
    size_type max_size() const throw()
    {
        return std::numeric_limits<std::size_t>::max() / sizeof(T);
    }

    // allocate but don't initialize num elements of type T
    pointer allocate(size_type num, const void* = 0)
    {
        // print message and allocate memory with global new
        std::cerr << "allocate " << num << " element(s)"
                  << " of size " << sizeof(T) << std::endl;
        pointer ret = (pointer)(::operator new(num * sizeof(T)));
        std::cerr << " allocated at: " << (void*) ret << std::endl;
        return ret;
    }

    // initialize elements of allocated storage p with value value
    void construct(pointer p, const T& value)
    {
        // initialize memory with placement new
        new ((void*) p) T(value);
    }

    // destroy elements of initialized storage p
    void destroy(pointer p)
    {
        // destroy objects by calling their destructor
        p->~T();
    }

    // deallocate storage p of deleted elements
    void deallocate(pointer p, size_type num)
    {
        // print message and deallocate memory with global delete
        std::cerr << "deallocate " << num << " element(s)"
                  << " of size " << sizeof(T) << " at: " << (void*) p
                  << std::endl;
        ::operator delete((void*) p);
    }
};

// return that all specializations of this allocator are interchangeable
template <class T1, class T2>
bool operator==(const MyAlloc<T1>&, const MyAlloc<T2>&) throw()
{
    return true;
}
template <class T1, class T2>
bool operator!=(const MyAlloc<T1>&, const MyAlloc<T2>&) throw()
{
    return false;
}

class nbrowser: public std::enable_shared_from_this<nbrowser> {
  public:
    nbrowser(boost::asio::io_context& ctx)
        : record_(MyAlloc<char>())
        , browser_(ctx)
        , resolver_(ctx)
    {
    }

    void run()
    {
        boost::system::error_code ec;

        // open browser for http services on all interfaces
        browser_.open(boost::asio::dnssd::network_interface::all(),
                      boost::asio::dnssd::tcp_service::type("_printer"),
                      "local", ec);

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

        browser_.async_browse(record_, [self](boost::system::error_code ec) {
            std::cout << "handler called: " << self->record_.fqdn() << "\n";
        });
    }

    boost::asio::dnssd::basic_service_record<MyAlloc<char>> record_;
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
