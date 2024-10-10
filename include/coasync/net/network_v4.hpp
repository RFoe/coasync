/// network_v4/network_v6

#ifndef COASYNC_NETWORK_V4_INCLUDED
#define COASYNC_NETWORK_V4_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "address_v4.hpp"
#include "address_v4_iterator.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
struct [[nodiscard]] network_v4
{
  typedef int 				prefix_length_type;
  typedef address_v4 	address_type;

  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit network_v4() noexcept
    : _M_addr()
    , _M_prefix_len(0) {}
  COASYNC_ATTRIBUTE((always_inline))
  constexpr network_v4(address_v4 const& addr, int prefix_len) noexcept(false)
    : _M_addr(addr)
    , _M_prefix_len(prefix_len)
  {
    if (_M_prefix_len < 0 || _M_prefix_len > 32) COASYNC_ATTRIBUTE((unlikely))
      throw std::out_of_range("network_v4: invalid prefix length");
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr network_v4(address_v4 const& addr, address_v4 const& mask) noexcept(false)
    : _M_addr(addr)
    , _M_prefix_len(std::popcount(mask.to_uint()))
  {
    if(_M_prefix_len != 0) COASYNC_ATTRIBUTE((likely))
      {
        address_v4::uint_type mask_uint = mask.to_uint();
        if (std::countr_zero(mask_uint) != (32 - _M_prefix_len))
          throw std::invalid_argument("network_v4: invalid mask");
        if ((mask_uint & 0x80000000) == 0)
          throw std::invalid_argument("network_v4: invalid mask");
      }
  }
  COASYNC_ATTRIBUTE((always_inline))
  ~ network_v4() noexcept
  {
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr network_v4(network_v4 const&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr network_v4& operator=(network_v4 const&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr network_v4(network_v4&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr network_v4& operator=(network_v4&&) noexcept = default;
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) /*constexpr*/ address_v4 address() const noexcept
  {
    return _M_addr;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) constexpr int prefix_length() const noexcept
  {
    return _M_prefix_len;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  /*constexpr */address_v4 netmask() const noexcept
  {
    COASYNC_ATTRIBUTE((gnu::uninitialized)) address_v4 __m;
    if (_M_prefix_len) COASYNC_ATTRIBUTE((likely))
      __m = address_v4(0xFFFFFFFFu << (32 - _M_prefix_len));
    return __m;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  /*constexpr*/ address_v4 network() const noexcept
  {
    return address_v4{_M_addr.to_uint()& netmask().to_uint()};
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  /*constexpr*/ address_v4 broadcast() const noexcept
  {
    auto __b = _M_addr.to_uint();
    if (_M_prefix_len < 32) COASYNC_ATTRIBUTE((likely))
      __b |= 0xFFFFFFFFu >> _M_prefix_len;
    return address_v4{__b};
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  address_v4_range hosts() const noexcept
  {
    if (is_host()) COASYNC_ATTRIBUTE((unlikely))
      return address_v4_range{ address(), *++address_v4_iterator(address()) };
    return address_v4_range{ network(), broadcast() };
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  /*constexpr */network_v4 canonical() const noexcept
  {
    return network_v4(network(), prefix_length());
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) constexpr bool is_host() const noexcept
  {
    return _M_prefix_len == 32;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  /*constexpr */bool is_subnet_of(const network_v4& __other) const noexcept
  {
    if (__other.prefix_length() < prefix_length()) COASYNC_ATTRIBUTE((likely))
      {
        network_v4 __net(address(), __other.prefix_length());
        return __net.canonical() == __other.canonical();
      }
    return false;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend /*constexpr*/ bool operator==(const network_v4& __a, const network_v4& __b) noexcept
  {
    return __a.address() == __b.address()
           && __a.prefix_length() == __b.prefix_length();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend /*constexpr*/ bool operator!=(const network_v4& __a, const network_v4& __b) noexcept
  {
    return !(__a == __b);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  std::string to_string() const noexcept
  {
    std::string __str = address().to_string();
    const unsigned __addrlen = __str.length();
    const unsigned __preflen = prefix_length() >= 10 ? 2 : 1;
    auto __write = [__addrlen, __preflen, this]
#if __cplusplus >= 202207L
                   COASYNC_ATTRIBUTE((nodiscard, always_inline))
#endif
                   (char* __p, size_t __n)
    {
      __p[__addrlen] = '/';
      __to_chars_10_impl(__p + __addrlen + 1, __preflen, (unsigned char)prefix_length());
      return __n;
    };
    const unsigned __len = __addrlen + 1 + __preflen;
#if __cpp_lib_string_resize_and_overwrite
    __str.resize_and_overwrite(__len, __write);
#else
    __str.resize(__len);
    __write(&__str.front(), __len);
#endif
    return __str;
  }
private:
  template<typename _Tp>
  constexpr void static __to_chars_10_impl(char* __first, unsigned __len, _Tp __val) noexcept COASYNC_ATTRIBUTE((gnu::nonnull))
  {
#if __cpp_variable_templates
    static_assert(std::is_integral_v<_Tp>, "implementation bug");
#endif

    constexpr char __digits[201] =
      "0001020304050607080910111213141516171819"
      "2021222324252627282930313233343536373839"
      "4041424344454647484950515253545556575859"
      "6061626364656667686970717273747576777879"
      "8081828384858687888990919293949596979899";
    unsigned __pos = __len - 1;
    while (__val >= 100)
      {
        auto const __num = (__val % 100) * 2;
        __val /= 100;
        __first[__pos] = __digits[__num + 1];
        __first[__pos - 1] = __digits[__num];
        __pos -= 2;
      }
    if (__val >= 10) COASYNC_ATTRIBUTE((unlikely))
      {
        auto const __num = __val * 2;
        __first[1] = __digits[__num + 1];
        __first[0] = __digits[__num];
      }
    else
      __first[0] = '0' + __val;
  }
  COASYNC_ATTRIBUTE((no_unique_address)) address_v4 _M_addr;
  int _M_prefix_len;
};

COASYNC_ATTRIBUTE((nodiscard, always_inline)) network_v4
make_network_v4(const address_v4& __a, int __prefix_len)
{
  return network_v4{__a, __prefix_len};
}

COASYNC_ATTRIBUTE((nodiscard, always_inline)) network_v4
make_network_v4(const address_v4& __a, const address_v4& __mask)
{
  return network_v4{ __a, __mask };
}

template<typename _CharT, typename _Traits>
COASYNC_ATTRIBUTE((nodiscard, always_inline)) std::basic_ostream<_CharT, _Traits>&
operator<<(std::basic_ostream<_CharT, _Traits>& __os, const network_v4& __net)
{
  return __os << __net.to_string();
}

}
}

#endif
