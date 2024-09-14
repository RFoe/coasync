#ifndef __COASYNC_ADDRESS_V6_INCLUDED
#define __COASYNC_ADDRESS_V6_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if defined(has_include)
# if defined(linux)
#  if has_include(<netinet/in.h>)
#   include <arpa/inet.h>
#  endif
#  if has_include(<arpa/inet.h>)
#   include <sys/socket.h>
#  endif
#  if has_include(<sys/socket.h>)
#   include <netinet/in.h>
#  endif
# endif
#endif
#include "../detail/networking.hpp"
#include "scope_id.hpp"
#include <algorithm>
#include <stdexcept>
#include <format>
#include <cstring>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
/// Implements IP version 6 style addresses.
/// The ip::address_v6 class provides the ability to use and manipulate IP version 6 addresses.
struct address_v6
{
	/// The scope ID type of address.
  typedef coasync::net::scope_id_type scope_id_type;
  /// The type used to represent an address as an array of bytes.
  struct bytes_type: public std::array<unsigned char, 16>
  {
    template <std::integral... Ts>
    constexpr explicit bytes_type(Ts... ts) noexcept : std::array<unsigned char, 16>
    {
      static_cast<unsigned char>(ts) ...
    }
    {
#if UCHAR_MAX > 0xFF
      for (auto b : *this)
        if (b > 0xFF) COASYNC_ATTRIBUTE((unlikely))
          throw std::out_of_range("invalid address_v6::bytes_type value");
#endif
    }
  };
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit address_v6() noexcept
    : M_bytes { 0 }, M_scope_id { 0 }
  {
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr address_v6(address_v6 const&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr address_v6& operator=(address_v6 const&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr address_v6(address_v6&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr address_v6& operator=(address_v6&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  ~ address_v6() noexcept
  {
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit address_v6(
    bytes_type const& bytes,
    scope_id_type scope_id = 0) noexcept
    : M_bytes(bytes), M_scope_id(scope_id) {}
  COASYNC_ATTRIBUTE((always_inline))
  constexpr void COASYNC_API scope_id(scope_id_type scope_id) noexcept
  {
    M_scope_id = scope_id;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr scope_id_type COASYNC_API scope_id() const noexcept
  {
    return M_scope_id;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_unspecified() const noexcept
  {
    bool const all_bytes_zero
      = std::ranges::all_of(M_bytes,
                            []
#if __cplusplus >= 202207L
                            COASYNC_ATTRIBUTE((nodiscard, always_inline))
#endif
                            (unsigned char value) noexcept -> bool
    {
      return 0 == value;
    });
    if (not all_bytes_zero) COASYNC_ATTRIBUTE((unlikely))
      return false;
    return 0 == M_scope_id;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_loopback() const noexcept
  {
    for (int __i = 0; __i < 15; ++__i)
      if (M_bytes[__i] != 0x00)
        return false;
    return M_bytes[15] == 0x01 && M_scope_id == 0;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_multicast() const noexcept
  {
    return 0xFF == M_bytes.front();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_link_local() const noexcept
  {
    return 0xFE == M_bytes.front() and 0x80 == (0xC0 & M_bytes[1]);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_site_local() const noexcept
  {
    return 0xFE == M_bytes.front() and 0xC0 == (0xC0 & M_bytes[1]);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_unique_local() const noexcept
  {
    return 0xFC == (M_bytes.front() & 0xFE) ;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_v4_mapped() const noexcept
  {
    bool const prefix_fit
      = std::all_of(M_bytes.cbegin(), M_bytes.cbegin() + 9,
                    []
#if __cplusplus >= 202207L
                    COASYNC_ATTRIBUTE((nodiscard, always_inline))
#endif
                    (unsigned char value) noexcept
    {
      return 0x00 == value;
    });
    return prefix_fit and 0xFF == M_bytes[10] and 0xFF == M_bytes[11];
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_multicast_node_local() const noexcept
  {
    return is_multicast() and 0x01 == (M_bytes[1] & 0x0F);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_multicast_link_local() const noexcept
  {
    return is_multicast() and 0x02 == (M_bytes[1] & 0x0F);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_multicast_site_local() const noexcept
  {
    return is_multicast() and 0x05 == (M_bytes[1] & 0x0F);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_multicast_org_local() const noexcept
  {
    return is_multicast() and 0x08  == (M_bytes[1] & 0x0F);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_multicast_unique_local() const noexcept
  {
    return is_multicast() and 0x0E  == (M_bytes[1] & 0x0F);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_multicast_global() const noexcept
  {
    return is_multicast() and 0x0B  == (M_bytes[1] & 0x0F);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bytes_type COASYNC_API to_bytes() const noexcept
  {
    return M_bytes;
  }
  COASYNC_ATTRIBUTE((nodiscard))
  std::string COASYNC_API to_string() const
  {
    std::string string;
    string.resize(INET6_ADDRSTRLEN + (M_scope_id ? 11 : 0));
    char* const pointer = std::addressof(string.front());
    if (::inet_ntop( AF_INET6, std::addressof(M_bytes[0]), pointer, string.size())) COASYNC_ATTRIBUTE((likely))
      {
        auto sentinel = string.find('\0');
        if (M_scope_id) COASYNC_ATTRIBUTE((unlikely))
          sentinel += std::sprintf(
                        pointer + sentinel,
                        "%%%lu",
                        static_cast<long unsigned int>(M_scope_id));
        string.erase(sentinel);
        return string;
      }
    else COASYNC_ATTRIBUTE((unlikely))
      {
        string.resize(0);
        return string;
      };
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  static address_v6 COASYNC_API any() noexcept
  {
    return address_v6 {};
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  static address_v6 COASYNC_API loopback() noexcept
  {
    return address_v6
    {
      bytes_type {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1}
    };
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend constexpr std::weak_ordering COASYNC_API operator<=> (address_v6 const& x, address_v6 const& y) noexcept
  {
    if (auto result = std::compare_three_way{}(x.M_bytes, y.M_bytes);
        result != std::weak_ordering::equivalent) COASYNC_ATTRIBUTE((likely))
      return result;
    return std::compare_three_way{}(x.M_scope_id, y.M_scope_id);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend constexpr bool operator== (address_v6 const& x, address_v6 const& y) noexcept
  {
    return x.M_bytes == y.M_bytes and x.M_scope_id == y.M_scope_id;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend constexpr bool COASYNC_API operator!= (address_v6 const& x, address_v6 const& y) noexcept
  {
    return x.M_bytes != y.M_bytes or x.M_scope_id != y.M_scope_id;
  }
private:
  template <typename _Tp>
  friend class std::hash;
  template <typename _AddressType>
  friend struct basic_address_iterator;
  bytes_type 	  	M_bytes;
  scope_id_type 	M_scope_id;
};
COASYNC_ATTRIBUTE((nodiscard))
inline address_v6 COASYNC_API make_address_v6(address_v6::bytes_type const& bytes, scope_id_type scope_id = 0) noexcept
{
  return address_v6 { bytes, scope_id };
}
COASYNC_ATTRIBUTE((nodiscard))
inline address_v6 COASYNC_API make_address_v6(char const* address, char const* scope, std::error_code& ec) noexcept
{
  address_v6::bytes_type bytes;
  const int result = ::inet_pton(AF_INET6, address, reinterpret_cast<void*>(std::addressof(bytes[0])));
  if (result > 0) COASYNC_ATTRIBUTE((likely))
    {
      ec.clear();
      if (not scope) COASYNC_ATTRIBUTE((likely))
        return address_v6 { bytes, 0 };
      char* sentinel = nullptr;
      unsigned long value = std::strtoul(scope, std::addressof(sentinel), 10);
      if (
        sentinel != scope and not* sentinel
        and value < std::numeric_limits<scope_id_type>::max()
      )
        return address_v6 { bytes, static_cast<scope_id_type>(value) };
      ec = std::make_error_code(std::errc::invalid_argument);
    }
  else if (0 == result) COASYNC_ATTRIBUTE((likely))
    ec = std::make_error_code(std::errc::invalid_argument);
  else COASYNC_ATTRIBUTE((unlikely))
    ec.assign(detail::get_errno(), detail::generic_category());
  return address_v6 {};
}
COASYNC_ATTRIBUTE((nodiscard))
inline address_v6 COASYNC_API make_address_v6(char const* addr_str, char const* scope)
{
  std::error_code ec;
  address_v6 address = make_address_v6(addr_str, scope, ec);
  if (ec) COASYNC_ATTRIBUTE((unlikely))
    throw std::system_error { ec, "make_address_v6" };
  return address;
}
COASYNC_ATTRIBUTE((nodiscard))
inline address_v6 COASYNC_API make_address_v6(char const* s, std::error_code& ec) noexcept
{
  auto 	scope_position = std::strchr(s, '%');
  if (not scope_position) COASYNC_ATTRIBUTE((likely))
    return make_address_v6(s, nullptr, ec);
  char buffer[64];
  std::size_t body_length = scope_position - s;
  std::strncpy(buffer, s, body_length);
  buffer[body_length] = '\0';
  return make_address_v6(buffer, scope_position + 1,
                         ec);
}
COASYNC_ATTRIBUTE((nodiscard))
inline address_v6 COASYNC_API make_address_v6(char const* s)
{
  std::error_code ec;
  address_v6 address = make_address_v6(s, ec);
  if (ec) COASYNC_ATTRIBUTE((unlikely))
    throw std::system_error { ec, "make_address_v6" };
  return address;
}
COASYNC_ATTRIBUTE((nodiscard))
inline address_v6 COASYNC_API make_address_v6(std::string const& s, std::error_code& ec) noexcept
{
  return make_address_v6(s.data(), ec);
}
COASYNC_ATTRIBUTE((nodiscard))
inline address_v6 COASYNC_API make_address_v6(std::string const& s)
{
  std::error_code ec;
  address_v6 address = make_address_v6(s, ec);
  if (ec) COASYNC_ATTRIBUTE((unlikely))
    throw std::system_error { ec, "make_address_v6" };
  return address;
}
COASYNC_ATTRIBUTE((nodiscard))
inline address_v6 COASYNC_API make_address_v6(std::string_view s, std::error_code& ec) noexcept
{
  return make_address_v6(s.data(), ec);
}
COASYNC_ATTRIBUTE((nodiscard))
inline address_v6 COASYNC_API make_address_v6(std::string_view s)
{
  std::error_code ec;
  address_v6 address = make_address_v6(s, ec);
  if (ec) COASYNC_ATTRIBUTE((unlikely))
    throw std::system_error { ec, "make_address_v6" };
  return address;
}
template <typename CharT, typename Traits>
COASYNC_ATTRIBUTE((maybe_unused, always_inline))
std::basic_ostream<CharT, Traits>& COASYNC_API operator<<(std::basic_ostream<CharT, Traits>& os, address_v6 const& a) noexcept
{
  return (os << a.to_string());
}
}
}
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) std
{
template<> struct hash<coasync::net::address_v6> final
{
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  std::size_t operator()(coasync::net::address_v6 const& address) const noexcept
  {
    std::size_t result = {};
    std::ranges::for_each(address.M_bytes, [&result]
#if __cplusplus >= 202207L
                          COASYNC_ATTRIBUTE((nodiscard, always_inline))
#endif
                          (unsigned char value) mutable noexcept -> void
    {
      result |= std::hash<int> {}(value);
    });
    result |= std::hash<coasync::net::scope_id_type> {}(address.M_scope_id);
    return result;
  }
};
template<>  struct formatter<coasync::net::address_v6>
{
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr auto parse(std::format_parse_context& context) const noexcept
  {
    return context.begin();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  auto format(coasync::net::address_v6 const& t, std::format_context& context) const noexcept
  {
    return std::format_to(context.out(), "{}", t.to_string());
  }
};
}
#endif
