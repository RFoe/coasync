/// network_v6/network_v6

#ifndef COASYNC_NETWORK_V6_INCLUDED
#define COASYNC_NETWORK_V6_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "address_v6.hpp"
#include "address_v6_iterator.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
struct [[nodiscard]] network_v6
{
  typedef int 				prefix_length_type;
  typedef address_v6 	address_type;

  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit network_v6() noexcept
    : _M_addr()
    , _M_prefix_len(0) {}
  COASYNC_ATTRIBUTE((always_inline))
  constexpr network_v6(address_v6 const& addr, int prefix_len) noexcept(false)
    : _M_addr(addr)
    , _M_prefix_len(prefix_len)
  {
    if (_M_prefix_len < 0 || _M_prefix_len > 128) COASYNC_ATTRIBUTE((unlikely))
      throw std::out_of_range("network_v6: invalid prefix length");
  }

  COASYNC_ATTRIBUTE((always_inline))
  ~ network_v6() noexcept
  {
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr network_v6(network_v6 const&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr network_v6& operator=(network_v6 const&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr network_v6(network_v6&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr network_v6& operator=(network_v6&&) noexcept = default;
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) /*constexpr*/ address_v6 address() const noexcept
  {
    return _M_addr;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) constexpr int prefix_length() const noexcept
  {
    return _M_prefix_len;
  }
  COASYNC_ATTRIBUTE((noreturn, always_inline)) /*constexpr*/ address_v6 network() const noexcept
  {
    COASYNC_TERMINATE();
  }

  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  address_v6_range hosts() const noexcept
  {
    if (is_host()) COASYNC_ATTRIBUTE((likely))
      return address_v6_range{ address(), *++address_v6_iterator(address()) };
    COASYNC_TERMINATE(); // { network(), XXX broadcast() XXX }; // TODO
  }

  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  /*constexpr*/ network_v6 canonical() const noexcept
  {
    return network_v6{network(), prefix_length()};
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool is_host() const noexcept
  {
    return _M_prefix_len == 128;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  /*constexpr*/ bool is_subnet_of(const network_v6& __other) const noexcept
  {
    if (__other.prefix_length() < prefix_length()) COASYNC_ATTRIBUTE((likely))
      {
        network_v6 __net(address(), __other.prefix_length());
        return __net.canonical() == __other.canonical();
      }
    return false;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  std::string to_string() const noexcept
  {
    return address().to_string() + '/' + std::to_string(prefix_length());
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend /*constexpr*/ bool operator==(const network_v6& __a, const network_v6& __b) noexcept
  {
    return __a.address() == __b.address()
           && __a.prefix_length() == __b.prefix_length();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend /*constexpr*/ bool operator!=(const network_v6& __a, const network_v6& __b) noexcept
  {
    return !(__a == __b);
  }
private:
  COASYNC_ATTRIBUTE((no_unique_address)) address_v6 _M_addr;
  int _M_prefix_len;
};

COASYNC_ATTRIBUTE((nodiscard, always_inline)) network_v6
make_network_v6(const address_v6& __a, int __prefix_len)
{
  return network_v6{__a, __prefix_len};
}

template<typename _CharT, typename _Traits>
COASYNC_ATTRIBUTE((nodiscard, always_inline)) std::basic_ostream<_CharT, _Traits>&
operator<<(std::basic_ostream<_CharT, _Traits>& __os, const network_v6& __net)
{
  return __os << __net.to_string();
}

}
}

#endif
