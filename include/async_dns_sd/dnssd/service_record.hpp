#pragma once

#include "network_interface.hpp"
#include <boost/asio.hpp>

namespace boost {
namespace asio {
namespace dnssd {

template <typename Allocator = std::allocator<char>>
class basic_service_record {
  public:
    using string_type
        = std::basic_string<char, std::char_traits<char>, Allocator>;
    using allocator = Allocator;

    basic_service_record()
        : if_idx_(-1)
    {
    }

    basic_service_record(const Allocator& alloc)
        : name_(alloc)
        , type_(alloc)
        , domain_(alloc)
    {
    }

    basic_service_record(network_interface::index_type if_index,
                         const string_type& name, const string_type& type,
                         const string_type& domain)
        : name_(name)
        , type_(type)
        , domain_(domain)
        , if_idx_(if_index)
    {
    }

    basic_service_record(basic_service_record<Allocator>&& other)
        : name_(std::move(other.name_))
        , type_(std::move(other.type_))
        , domain_(std::move(other.domain_))
        , if_idx_(other.if_idx_)
    {
        other.if_idx_ = -1;
    }

    template <typename OtherAllocator>
    void operator=(basic_service_record<OtherAllocator>&& other)
    {
        std::swap(name_, other.name_);
        std::swap(type_, other.type_);
        std::swap(domain_, other.domain_);
        if_idx_       = other.if_idx_;
        other.if_idx_ = -1;
    }

    string_type name() const
    {
        return name_;
    }

    void name(const string_type& name)
    {
        name_ = name;
    }

    void name(const char* name)
    {
        name_.assign(name);
    }

    string_type type() const
    {
        return type_;
    }

    void type(const string_type& type)
    {
        type_ = type;
    }

    void type(const char* type)
    {
        type_.assign(type);
    }

    string_type domain() const
    {
        return domain_;
    }

    void domain(const string_type& domain)
    {
        domain_ = domain;
    }

    void domain(const char* domain)
    {
        domain_.assign(domain);
    }

    network_interface::index_type if_index() const noexcept
    {
        return if_idx_;
    }

    void if_index(network_interface::index_type if_index) noexcept
    {
        if_idx_ = if_index;
    }

    network_interface interface() const
    {
        return network_interface(if_idx_);
    }

    string_type fqdn() const
    {
        string_type str(kDNSServiceMaxDomainName, '\0', name_.get_allocator());
        DNSServiceConstructFullName(
            &str[0], name_.c_str(), type_.c_str(), domain_.c_str());
        return str;
    }


  private:
    string_type name_;
    string_type type_;
    string_type domain_;
    network_interface::index_type if_idx_;
};

using service_record = basic_service_record<>;

}    // namespace dnssd
}    // namespace asio
}    // namespace boost
