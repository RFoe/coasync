#ifndef __COASYNC_ADDRESS_V4_INCLUDED
#define __COASYNC_ADDRESS_V4_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../detail/config.hpp"
#if defined(has_include)
# if defined(linux)
#  if has_include(<arpa/inet.h>)
#   include <arpa/inet.h>
#  endif
# endif
#endif
#include "../detail/networking.hpp"
#include <stdexcept>
#include <algorithm>
#include <format>
#include <cstdint>
#if !defined(cpp_lib_byteswap)
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) std
{
/// note: in GCC
/// Built-in Function: uint16_t __builtin_bswap16 (uint16_t x)
/// Built-in Function: uint32_t __builtin_bswap32 (uint32_t x)
/// Built-in Function: uint64_t __builtin_bswap64 (uint64_t x)
/// Built-in Function: uint128_t __builtin_bswap128 (uint128_t x)
/// note: in MSVC
/// _byteswap_uint64, _byteswap_ulong, _byteswap_ushort
/// note: in clang/llvm
/// llvm::byteswap
template<std::integral T>
COASYNC_ATTRIBUTE((nodiscard, always_inline))
constexpr T byteswap(T value) noexcept
{
  static_assert(std::has_unique_object_representations_v<T>);
  auto value_representation = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
  std::ranges::reverse(value_representation);
  /// An integer value of type T whose object representation comprises the bytes of that of n in reversed order.
  return std::bit_cast<T>(value_representation);
}
}
#endif
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
/// Implements IP version 4 style addresses.
/// The ip::address_v4 class provides the ability to use and manipulate IP version 4 addresses.
struct address_v4
{
  typedef std::uint_least32_t uint_type;
  /// The type used to represent an address as an unsigned integer.
  struct bytes_type: public std::array<unsigned char, sizeof(uint_type)>
  {
  	/// The type used to represent an address as an array of bytes.
    template <std::integral... Ts>
    constexpr explicit bytes_type(Ts... ts) noexcept
      : std::array<unsigned char, sizeof(uint_type)>
    {
      static_cast<unsigned char>(ts) ...
    }
    {
#if UCHAR_MAX > 0xFF
      for (auto b : *this)
        if (b > 0xFF) COASYNC_ATTRIBUTE((unlikely))
          throw std::out_of_range("invalid address_v4::bytes_type value");
#endif
    }
  };
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit address_v4()
  noexcept: M_address(0)
  {
  }
  COASYNC_ATTRIBUTE((always_inline))
  ~ address_v4() noexcept
  {
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr address_v4(address_v4 const&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr address_v4& operator=(address_v4 const&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr address_v4(address_v4&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr address_v4& operator=(address_v4&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit address_v4(bytes_type const& bytes) noexcept:
    M_address(std::bit_cast<std::uint_least32_t>(bytes))
  {
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit address_v4(std::uint_least32_t value) noexcept
    : M_address(std::byteswap(value))
  {
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr std::uint_least32_t COASYNC_API to_uint() const noexcept
  {
    return std::byteswap(M_address);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bytes_type COASYNC_API to_bytes() const noexcept
  {
    return std::bit_cast<bytes_type>(M_address);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_unspecified() const noexcept
  {
    return (0 == M_address);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_loopback() const noexcept
  {
    return (to_uint() & 0xFF000000) == 0x7F000000;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_multicast() const noexcept
  {
    return (to_uint() & 0xF0000000) == 0xE0000000;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  std::string COASYNC_API to_string() const
  {
    std::uint_least32_t value { std::byteswap(M_address) };
    std::string string {""};
    string.reserve(15);
#if __cpp_lib_string_resize_and_overwrite
/// Resizes the string to contain at most count characters, using the user-provided
/// operation op to modify the possibly indeterminate contents and set the length.
/// This avoids the cost of initializing a suitably-sized std::string when it is
/// intended to be used as a char array to be populated by, e.g., a C API call.
#endif
    string += std::to_string((value >> 24) & 0xFF);
    string += '.';
    string += std::to_string((value >> 16) & 0xFF);
    string += '.';
    string += std::to_string((value >> 8) & 0xFF);
    string += '.';
    string += std::to_string((value) & 0xFF);
    string.shrink_to_fit();
    return string;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  static address_v4 COASYNC_API any() noexcept
  {
    return address_v4{};
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  static address_v4 COASYNC_API loopback() noexcept
  {
    return address_v4{0x7F000001};
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  static address_v4 COASYNC_API broadcast() noexcept
  {
    return address_v4{0xFFFFFFFF};
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend constexpr std::weak_ordering COASYNC_API operator<=>(address_v4 const& x, address_v4 const& y) noexcept
  {
    return std::compare_three_way{}(x.to_uint(), y.to_uint());
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend constexpr bool COASYNC_API operator== (address_v4 const& x, address_v4 const& y) noexcept
  {
    return x.to_uint() == y.to_uint();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend constexpr bool COASYNC_API operator!=(address_v4 const& x, address_v4 const& y) noexcept
  {
    return x.to_uint() != y.to_uint();
  }
private:
  friend address_v4 make_address_v4(char const* s, std::error_code& ec) noexcept;
  std::uint32_t M_address;
};
COASYNC_ATTRIBUTE((nodiscard))
address_v4 COASYNC_API make_address_v4(address_v4::bytes_type const& bytes) noexcept
{
  return address_v4{ bytes };
}
COASYNC_ATTRIBUTE((nodiscard))
address_v4 COASYNC_API make_address_v4(address_v4::uint_type value) noexcept
{
  return address_v4{ value };
}
COASYNC_ATTRIBUTE((nodiscard))
address_v4 COASYNC_API make_address_v4(char const* str, std::error_code& ec) noexcept
{
  address_v4 address;
  const int result = ::inet_pton(AF_INET, str, reinterpret_cast<void*>(std::addressof(address.M_address)));
  if (result > 0) COASYNC_ATTRIBUTE((likely))
    {
      ec.clear();
      return address;
    }
  if (result == 0) COASYNC_ATTRIBUTE((unlikely))
    ec = std::make_error_code(std::errc::invalid_argument);
  else COASYNC_ATTRIBUTE((likely))
    ec.assign(detail::get_errno(), detail::generic_category());
  return address_v4 {};
}
COASYNC_ATTRIBUTE((nodiscard))
address_v4 COASYNC_API make_address_v4(char const* str)
{
  std::error_code ec;
  address_v4 address = make_address_v4(str, ec);
  if (ec) COASYNC_ATTRIBUTE((unlikely))
    throw std::system_error { ec, "make_address_v4" };
  return address;
}
COASYNC_ATTRIBUTE((nodiscard))
address_v4 COASYNC_API make_address_v4(std::string const& str, std::error_code& e) noexcept
{
  return make_address_v4(str.c_str(), e);
}
COASYNC_ATTRIBUTE((nodiscard))
address_v4 COASYNC_API make_address_v4(std::string const& s)
{
  std::error_code ec;
  address_v4 address = make_address_v4(s, ec);
  if (ec) COASYNC_ATTRIBUTE((unlikely))
    throw std::system_error { ec, "make_address_v4" };
  return address;
}
COASYNC_ATTRIBUTE((nodiscard))
address_v4 COASYNC_API make_address_v4(std::string_view s, std::error_code& e) noexcept
{
  char buffer[16] = {0};
  std::size_t length = s.copy(buffer, std::size(buffer));
  if (length == std::size(buffer)) COASYNC_ATTRIBUTE((unlikely))
    {
      e = std::make_error_code(std::errc::invalid_argument);
      return address_v4 {};
    }
  e.clear();
  buffer[length] = '\0';
  return make_address_v4(buffer, e);
}
COASYNC_ATTRIBUTE((nodiscard))
address_v4 COASYNC_API make_address_v4(std::string_view s)
{
  std::error_code ec;
  address_v4 address = make_address_v4(s, ec);
  if (ec) COASYNC_ATTRIBUTE((unlikely))
    throw std::system_error { ec, "make_address_v4" };
  return address;
}
template<typename CharT, typename Traits>
COASYNC_ATTRIBUTE((maybe_unused, always_inline))
std::basic_ostream<CharT, Traits>& COASYNC_API operator<<(std::basic_ostream<CharT, Traits>& os, address_v4 const& a) noexcept
{
  return (os << a.to_string());
}
}
}
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) std
{
template<> struct hash<coasync::net::address_v4> final
{
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  std::size_t operator()( coasync::net::address_v4 const& address) const noexcept
  {
    return std::hash<std::uint_least32_t> {}(address.to_uint());
  }
};
template<>  struct formatter<coasync::net::address_v4>
{
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr auto parse(std::format_parse_context& context) const noexcept
  {
    return context.begin();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  auto format(coasync::net::address_v4 const& t, std::format_context& context) const noexcept
  {
    return std::format_to(context.out(), "{}", t.to_string());
  }
};
}
#endif
