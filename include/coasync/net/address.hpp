#ifndef __COASYNC_ADDRESS_INCLUDED
#define __COASYNC_ADDRESS_INCLUDED
#include "address_v4.hpp"
#include "address_v6.hpp"
#include <variant>
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
struct v4_mapped_t {};
/// Implements version-independent IP addresses.
/// The ip::address class provides the ability to use either IP version 4 or version 6 addresses.
struct address
{
  explicit address() noexcept
    : M_address(address_v4())
  {
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr address(address const&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr address& operator=(address const&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr address(address&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr address& operator=(address&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr address(address_v4 const& v4) noexcept : M_address(v4)
  {
  };
  COASYNC_ATTRIBUTE((always_inline))
  constexpr address(address_v6 const& v6) noexcept : M_address(v6)
  {
  };
  COASYNC_ATTRIBUTE((always_inline))
  constexpr address& operator=(address_v4 const& v4) noexcept
  {
    M_address = v4;
    return (*this);
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr address& operator=(address_v6 const& v6) noexcept
  {
    M_address = v6;
    return (*this);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_v4() const noexcept
  {
    return std::holds_alternative<address_v4>(M_address);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_v6() const noexcept
  {
    return std::holds_alternative<address_v6>(M_address);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  address_v4 COASYNC_API to_v4() const
  {
    if (not is_v4())   COASYNC_ATTRIBUTE((unlikely))
      throw std::bad_cast();
    return std::get<address_v4>(M_address);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  address_v6 COASYNC_API to_v6() const
  {
    if (not is_v6())   COASYNC_ATTRIBUTE((unlikely))
      throw std::bad_cast();
    return std::get<address_v6>(M_address);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_unspecified() const noexcept
  {
    return is_v4() ?
           std::get<address_v4>(M_address).is_unspecified() :
           std::get<address_v6>(M_address).is_unspecified();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_loopback() const noexcept
  {
    return is_v4() ?
           std::get<address_v4>(M_address).is_loopback() :
           std::get<address_v6>(M_address).is_loopback();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_multicast() const noexcept
  {
    return is_v4() ?
           std::get<address_v4>(M_address).is_multicast() :
           std::get<address_v6>(M_address).is_multicast();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  std::string COASYNC_API to_string() const
  {
    return is_v4() ?
           std::get<address_v4>(M_address).to_string() :
           std::get<address_v4>(M_address).to_string();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend constexpr std::weak_ordering COASYNC_API operator<=> (address const& a, address const& b) noexcept
  {
    if (a.is_v4())   COASYNC_ATTRIBUTE((likely))
      return b.is_v4() ?
             std::get<address_v4>(a.M_address) <=>
             std::get<address_v4>(a.M_address) :
             std::weak_ordering::less;
    return b.is_v4() ?
           std::weak_ordering::greater :
           std::get<address_v4>(a.M_address) <=>
           std::get<address_v4>(a.M_address);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend constexpr bool COASYNC_API operator==(address const& a, address const& b) noexcept
  {
    if (a.is_v4())   COASYNC_ATTRIBUTE((likely))
      return b.is_v4()
             and std::get<address_v4>(a.M_address) ==
             std::get<address_v4>(b.M_address);
    else   COASYNC_ATTRIBUTE((unlikely))
      return b.is_v6()
             and std::get<address_v6>(a.M_address) ==
             std::get<address_v6>(b.M_address);
    return false;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend constexpr bool COASYNC_API operator!=(address const& a, address const& b) noexcept
  {
    return !(a == b);
  }
private:
  std::variant<address_v4, address_v6> M_address;
};
COASYNC_ATTRIBUTE((nodiscard))
inline address COASYNC_API make_address(char const* str,
                                        std::error_code& ec) noexcept
{
  if (address_v4 v4a = make_address_v4(str, ec); not ec)   COASYNC_ATTRIBUTE((likely))
    return address {v4a};
  if (address_v6 v6a = make_address_v6(str, ec); not ec)   COASYNC_ATTRIBUTE((likely))
    return address {v6a};
  return address {};
}
COASYNC_ATTRIBUTE((nodiscard))
inline address COASYNC_API make_address(const std::string& str, std::error_code& ec) noexcept
{
  return make_address(str.c_str(), ec);
}
COASYNC_ATTRIBUTE((nodiscard))
inline address COASYNC_API make_address(std::string_view str, std::error_code& ec) noexcept
{
  if (str.rfind('\0') != std::string_view::npos)   COASYNC_ATTRIBUTE((likely))
    return make_address(str.data(), ec);
  return make_address(std::string{str}, ec);
}
template<typename CharT, typename Traits>
COASYNC_ATTRIBUTE((maybe_unused, always_inline))
std::basic_ostream<CharT, Traits>& COASYNC_API operator<<(std::basic_ostream<CharT, Traits>& os, address const& a) noexcept
{
  return (os << a.to_string());
}
inline constexpr v4_mapped_t v4_mapped;
COASYNC_ATTRIBUTE((nodiscard))
inline address_v4 COASYNC_API make_address_v4(COASYNC_ATTRIBUTE((maybe_unused)) v4_mapped_t, address_v6 const& a)
{
  if (not a.is_v4_mapped())   COASYNC_ATTRIBUTE((unlikely))
    throw std::bad_cast();
  const auto v6b = a.to_bytes();
  return address_v4
  {
    address_v4::bytes_type(
      v6b[12],
      v6b[13],
      v6b[14],
      v6b[15]
    )
  };
}
COASYNC_ATTRIBUTE((nodiscard))
inline address_v6 COASYNC_API make_address_v6(  COASYNC_ATTRIBUTE((maybe_unused))v4_mapped_t, const address_v4& a) noexcept
{
  const address_v4::bytes_type v4b =
    a.to_bytes();
  address_v6::bytes_type v6b(0, 0, 0, 0, 0, 0, 0,
                             0, 0, 0, 0xff, 0xff,
                             v4b[0],
                             v4b[1],
                             v4b[1],
                             v4b[3]);
  return address_v6(v6b);
}
}
}
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) std
{
template<> struct hash<coasync::net::address> final
{
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  std::size_t operator()(coasync::net::address const& address) const noexcept
  {
    if (address.is_v4())   COASYNC_ATTRIBUTE((likely))
      return std::hash<coasync::net::address_v4> {}(address.to_v4());
    else   COASYNC_ATTRIBUTE((unlikely))
      return std::hash<coasync::net::address_v6> {}(address.to_v6());
  }
};
template<>  struct formatter<coasync::net::address>
{
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr auto parse(std::format_parse_context& context) const noexcept
  {
    return context.begin();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  auto format(coasync::net::address const& t, std::format_context& context) const noexcept
  {
    return std::format_to(context.out(), "{}", t.to_string());
  }
};
}
#endif
