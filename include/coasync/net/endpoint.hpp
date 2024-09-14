#ifndef __COASYNC_ENDPOINT_INCLUDED
#define __COASYNC_ENDPOINT_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "address.hpp"
#include "port.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
/// The ip::basic_endpoint class template describes an endpoint that may be
/// associated with a particular socket.
template <typename Protocol>
struct basic_endpoint
{
  typedef Protocol protocol_type;
  typedef coasync::net::address address_type;
  typedef coasync::net::port_type port_type;
  typedef coasync::net::address_v4 address_v4_type;
  typedef coasync::net::address_v6 address_v6_type;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit basic_endpoint() noexcept
  {
  };
  COASYNC_ATTRIBUTE((always_inline))
  constexpr basic_endpoint(basic_endpoint const&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr basic_endpoint& operator=(basic_endpoint const&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr basic_endpoint(basic_endpoint&&)noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr basic_endpoint& operator=(basic_endpoint&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  ~ basic_endpoint() noexcept
  {
  };
  COASYNC_ATTRIBUTE((always_inline))
  constexpr basic_endpoint(
    protocol_type const& 	proto,
    port_type 						port = port_type()
  ) noexcept
    : M_port(std::byteswap<port_type>(port))
  {
    if (proto == protocol_type::v6()) COASYNC_ATTRIBUTE((unlikely))
      M_address = address_v6();
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr basic_endpoint(
    address_type const& 	address,
    port_type 						port = port_type()
  ) noexcept
    : M_address(address)
    , M_port(std::byteswap(port))
  {
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr protocol_type COASYNC_API protocol() const noexcept
  {
    return M_address.is_v4() ? protocol_type::v4() :
           protocol_type::v6() ;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr address_type COASYNC_API address() const noexcept
  {
    return M_address;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr port_type COASYNC_API port() const noexcept
  {
    return std::byteswap(M_port);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  ::sockaddr_in COASYNC_API to_sockaddr_in() const noexcept
  {
    sockaddr_in __sockaddr;
    typename address_v4_type::bytes_type bytes = address().to_v4().to_bytes();
    __sockaddr.sin_family = AF_INET;
    __sockaddr.sin_port = ::htons(port());
    std::memcpy(
      std::addressof(__sockaddr.sin_addr.s_addr), std::addressof(bytes),
      sizeof(typename address_v4_type::bytes_type));
    return __sockaddr;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  ::sockaddr_in6 COASYNC_API to_sockaddr_in6() const noexcept
  {
    sockaddr_in6 __sockaddr6;
    typename address_v6_type::bytes_type bytes = address().to_v6().to_bytes();
    __sockaddr6.sin6_family = AF_INET6;
    __sockaddr6.sin6_port = ::htons(port());
    __sockaddr6.sin6_scope_id = address().to_v6().scope_id();
    std::memcpy(
      std::addressof(__sockaddr6.sin6_addr.s6_addr), std::addressof(bytes),
      sizeof(typename address_v6_type::bytes_type));
    return __sockaddr6;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  static basic_endpoint COASYNC_API from_sockaddr_in(::sockaddr_in const& __sockaddr) noexcept
  {
    return basic_endpoint
    {
      address_v4_type {
        std::bit_cast<typename address_v4_type::bytes_type>(__sockaddr.sin_addr.s_addr)
      },
      port_type { ::ntohs(__sockaddr.sin_port) }
    };
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  static basic_endpoint COASYNC_API from_sockaddr_in6(::sockaddr_in6 const& __sockaddr6) noexcept
  {
    return basic_endpoint
    {
      address_v6_type {
        std::bit_cast<typename address_v6_type::bytes_type>(__sockaddr6.sin6_addr.s6_addr),
        scope_id_type { __sockaddr6.sin6_scope_id }
      },
      port_type { ::ntohs(__sockaddr6.sin6_port) }
    };
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr void COASYNC_API address(address_type const& address) noexcept
  {
    M_address = address;
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr void COASYNC_API port(port_type port) noexcept
  {
    M_port = std::byteswap(port);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend constexpr std::weak_ordering COASYNC_API operator<=> (basic_endpoint const& x, basic_endpoint const& y) noexcept
  {
    if (x.address() != y.address()) COASYNC_ATTRIBUTE((unlikely))
      return x.address() <=> y.address();
    return std::compare_three_way{} (x.port(),
                                     y.port());
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend constexpr COASYNC_API bool operator==(basic_endpoint const& a, basic_endpoint const& b) noexcept
  {
    return a.address() == b.address() and a.port() == b.port();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend constexpr COASYNC_API bool operator!=(basic_endpoint const& a, basic_endpoint const& b) noexcept
  {
    return not (a == b);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr std::string to_string()
  {
    if (M_address.is_v6()) COASYNC_ATTRIBUTE((unlikely))
      return "[" + M_address.to_string() + "]" + ":" + std::to_string(port());
    return M_address.to_string() + ":" + std::to_string(port());
  }
private:
  address_type 	M_address;
  port_type 				M_port;
};
template<typename Protocol, typename CharT, typename Traits>
COASYNC_ATTRIBUTE((maybe_unused, always_inline))
std::basic_ostream<CharT, Traits>& COASYNC_API
operator<<(
  std::basic_ostream<CharT, Traits>& os,
  basic_endpoint<Protocol> const& p
) noexcept
{
  if (p.address().is_v6()) COASYNC_ATTRIBUTE((unlikely))
    return (os << "[" << p.address() << "]" << ":" << p.port());
  return (os << p.address() << ":" << p.port());
}
}
}
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) std
{
template<typename Protocol> struct hash<coasync::net::basic_endpoint<Protocol>> final
{
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  std::size_t operator()(coasync::net::basic_endpoint<Protocol> const& endpoint) const noexcept
  {
    return std::hash<coasync::net::address> {}(endpoint.address())
           | std::hash<coasync::net::port_type> {}(endpoint.port());
  }
};
template<typename Protocol>  struct formatter<coasync::net::basic_endpoint<Protocol>>
{
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr auto parse(std::format_parse_context& context) const noexcept
  {
    return context.begin();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  auto format(coasync::net::basic_endpoint<Protocol> const& t, std::format_context& context) const noexcept
  {
    return std::format_to(context.out(), "{}", t.to_string());
  }
};
}
#endif
