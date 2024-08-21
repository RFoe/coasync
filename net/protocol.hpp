#ifndef __COASYNC_PROTOCOL_INCLUDED
#define __COASYNC_PROTOCOL_INCLUDED
#include "../detail/config.h"
#if defined(__has_include)
# if (defined(_WIN32)  || defined(_WIN64)) && __has_include(<ws2tcpip.h>)
#  include <ws2tcpip.h>
# elif defined(linux) && __has_include(<sys/socket.h>)
#  include <sys/socket.h>
# endif
#endif
namespace [[gnu::visibility("default")]] coasync
{
struct execution_context;
namespace [[gnu::visibility("default")]] net
{
enum class address_family : int {};
enum class socket_category : int {};
enum class internet_protocol : int {};
/// IP  is  the transport layer protocol used by the	Internet protocol family.
/// Options may be set	at the IP level	when using higher-level	protocols that
/// are based on IP (such as TCP and UDP).	 It may	 also  be  accessed  through
/// a  "raw	socket"	when developing	new protocols, or special-purpose applications.
inline constexpr address_family ipv4 = static_cast<address_family>(AF_INET);
inline constexpr address_family ipv6 = static_cast<address_family>(AF_INET6);
inline constexpr address_family unspec = static_cast<address_family>(AF_UNSPEC);
inline constexpr socket_category stream_socket = static_cast<socket_category>(SOCK_STREAM);
inline constexpr socket_category datagram_socket = static_cast<socket_category>(SOCK_DGRAM);
inline constexpr internet_protocol tcp_protocol = static_cast<internet_protocol>(IPPROTO_TCP);
inline constexpr internet_protocol udp_protocol = static_cast<internet_protocol>(IPPROTO_UDP);
inline constexpr std::size_t ipv4_strlen = INET_ADDRSTRLEN;
inline constexpr std::size_t ipv6_strlen = INET6_ADDRSTRLEN;
template <typename _Protocol> 														struct basic_endpoint;
template <typename _Protocol, typename execution_context> struct __basic_socket;
template <typename _Protocol, typename execution_context> struct __basic_acceptor;
template <typename _Protocol, typename execution_context> struct __basic_resolver;
struct tcp
{
  typedef basic_endpoint<tcp> 											endpoint;
  typedef __basic_socket<tcp, execution_context> 		socket;
  typedef __basic_acceptor<tcp, execution_context> 	acceptor;
  typedef __basic_resolver<tcp, execution_context> 	resolver;
  constexpr tcp() = delete;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr tcp(tcp const&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr tcp& operator=(tcp const&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr tcp(tcp&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr tcp& operator=(tcp&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  ~ tcp() noexcept = default;
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  static constexpr tcp COASYNC_API v4() noexcept
  {
    return tcp(ipv4);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  static constexpr tcp COASYNC_API v6() noexcept
  {
    return tcp(ipv6);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr address_family COASYNC_API family() const noexcept
  {
    return M_family;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr socket_category COASYNC_API category() const noexcept
  {
    return stream_socket;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr internet_protocol COASYNC_API protocol() const noexcept
  {
    return tcp_protocol;
  }
private:
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit tcp(address_family family)
  noexcept	: M_family(family)
  {
  }
  address_family M_family;
};
COASYNC_ATTRIBUTE((nodiscard, always_inline))
constexpr bool operator==(const tcp& a, const tcp& b) noexcept
{
  return a.family() == b.family();
}
COASYNC_ATTRIBUTE((nodiscard, always_inline))
constexpr bool operator!=(const tcp& a, const tcp& b) noexcept
{
  return !(a == b);
}
struct udp
{
  typedef basic_endpoint<udp> 											endpoint;
  typedef __basic_socket<udp, execution_context> 		socket;
  typedef __basic_acceptor<udp, execution_context> 	acceptor;
  typedef __basic_resolver<udp, execution_context> 	resolver;
  constexpr udp() = delete;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr udp(udp const&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr udp& operator=(udp const&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr udp(udp&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr udp& operator=(udp&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  ~ udp() noexcept = default;
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  static constexpr udp COASYNC_API v4() noexcept
  {
    return udp(ipv4);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  static constexpr udp COASYNC_API v6() noexcept
  {
    return udp(ipv6);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr address_family COASYNC_API family() const noexcept
  {
    return M_family;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr socket_category COASYNC_API category() const noexcept
  {
    return datagram_socket;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr internet_protocol COASYNC_API protocol() const noexcept
  {
    return udp_protocol;
  }
private:
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit udp(address_family family) noexcept
    : M_family(family)
  {
  }
  address_family M_family;
};
COASYNC_ATTRIBUTE((nodiscard, always_inline))
constexpr bool operator==(const udp& a, const udp& b) noexcept
{
  return a.family() == b.family();
}
COASYNC_ATTRIBUTE((nodiscard, always_inline))
constexpr bool operator!=(const udp& a, const udp& b) noexcept
{
  return !(a == b);
}
}
}
#endif
