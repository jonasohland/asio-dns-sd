#pragma once

#include <boost/asio/posix/descriptor.hpp>

namespace boost {
namespace asio {
namespace dnssd {

template <typename Executor>
class waitable_descriptor: public posix::basic_descriptor<Executor> {
  public:
    /**
     * @brief Construct a new waitable descriptor object
     * @param exec Executor the wait operations will take place on
     */
    explicit waitable_descriptor(const Executor& exec)
        : posix::basic_descriptor<Executor>(exec)
    {
    }

    /**
     * @brief Construct a new waitable descriptor object
     *
     * @tparam ExecutionContext
     * @param context Execution context the io operations will take place on
     */
    template <typename ExecutionContext>
    explicit waitable_descriptor(
        ExecutionContext& context,
        typename enable_if<is_convertible<
            ExecutionContext&, execution_context&>::value>::type* = 0)
        : posix::basic_descriptor<Executor>(context)
    {
    }

    /**
     * @brief default desctructor
     */
    ~waitable_descriptor()
    {
    }
    
private:
    bool still_readable_ = false;
};

}    // namespace dnssd
}    // namespace asio
}    // namespace boost
