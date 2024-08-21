#ifndef __COASYNC_OPTION_INCLUDED
#define __COASYNC_OPTION_INCLUDED
#if defined(__has_include)
# if defined(_WIN32) || defined(_WIN64)
#  if __has_include(<ws2tcpip.h>)
#   include <ws2tcpip.h>
#  endif
# elif defined(linux)
#  if __has_include <sys/socket.h>
#   include <sys/socket.h>
#  endif
# endif
#endif
#include "address.hpp"
#include <utility>
#include <chrono>
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
struct no_data_type {};
/// Socket options determine the behavior of the current Socket. Set optionValue to
/// true to enable the option, or to false to disable the option.
/// Socket options are grouped by level of protocol support.
template <typename crtp_object, typename data_type = no_data_type>
struct basic_option
{
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr int COASYNC_API level(
    COASYNC_ATTRIBUTE((maybe_unused)) auto const& protocol) const noexcept
  {
    return crtp_object::static_level;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr int COASYNC_API name(
    COASYNC_ATTRIBUTE((maybe_unused)) auto const& protocol) const noexcept
  {
    return crtp_object::static_name;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr char const* COASYNC_API data(
    COASYNC_ATTRIBUTE((maybe_unused)) auto const& protocol) const noexcept
  {
    return reinterpret_cast<char const*>(std::addressof(value));
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr char* COASYNC_API data(
    COASYNC_ATTRIBUTE((maybe_unused)) auto const& protocol) noexcept
  {
    return reinterpret_cast<char*>(std::addressof(value));
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr ::socklen_t  COASYNC_API size(
    COASYNC_ATTRIBUTE((maybe_unused)) auto const& protocol) const noexcept
  {
    return sizeof(data_type);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr data_type& COASYNC_API get() noexcept
  {
    return value;
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit basic_option(data_type value) noexcept: value(value)
  {
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit basic_option() noexcept: value {}
  {
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr basic_option& operator=(data_type value) noexcept
  {
    return (value = value, (*this));
  }
protected:
  COASYNC_ATTRIBUTE((no_unique_address)) data_type value;
};
struct socket_option final
{
  struct broadcast final : public basic_option<broadcast, bool>
  {
  public:
    using basic_option::basic_option;
    using basic_option::operator=;
  private:
    friend basic_option<broadcast, bool>;
    inline static constexpr int static_level =
      SOL_SOCKET;
    inline static constexpr int static_name =
      SO_BROADCAST;
  };
  struct  debug : public basic_option<debug, bool>
  {
  public:
    friend basic_option<debug, bool>;
    using basic_option::basic_option;
    using basic_option::operator=;
  private:
    inline static constexpr int static_level = SOL_SOCKET;
    inline static constexpr int static_name = SO_DEBUG;
  };
  struct  error : public basic_option<error, int>
  {
  public:
    friend basic_option<error, int>;
    using basic_option::basic_option;
    using basic_option::operator=;
  private:
    inline static constexpr int static_level = SOL_SOCKET;
    inline static constexpr int static_name = SO_ERROR;
  };
  struct  do_not_route : public basic_option<do_not_route, bool>
  {
  public:
    using basic_option::basic_option;
    using basic_option::operator=;
  private:
    friend basic_option<do_not_route, bool>;
    inline static constexpr int static_level =
      SOL_SOCKET;
    inline static constexpr int static_name =
      SO_DONTROUTE;
  };
  struct  keep_alive : public basic_option<keep_alive, bool>
  {
  public:
    using basic_option::basic_option;
    using basic_option::operator=;
  private:
    friend basic_option<keep_alive, bool>;
    inline static constexpr int static_level = SOL_SOCKET;
    inline static constexpr int static_name = SO_KEEPALIVE;
  };
  struct  linger : public basic_option<linger, ::linger>
  {
  public:
    using basic_option::basic_option;
    using basic_option::operator=;
    COASYNC_ATTRIBUTE((always_inline))
    constexpr linger() noexcept = default;
    inline linger(bool e, std::chrono::seconds t) noexcept
    {
      enabled(e);
      timeout(t);
    }
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    bool enabled() const noexcept
    {
      return value.l_onoff != 0;
    }
    COASYNC_ATTRIBUTE((always_inline))
    void enabled(bool e) noexcept
    {
      value.l_onoff = int(e);
    }
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    std::chrono::seconds timeout() const noexcept
    {
      return std::chrono::seconds(value.l_linger);
    }
    COASYNC_ATTRIBUTE((always_inline))
    void timeout(std::chrono::seconds t) noexcept
    {
      value.l_linger = t.count();
    }
  private:
    friend basic_option<linger, ::linger>;
    inline static constexpr int static_level = SOL_SOCKET;
    inline static constexpr int static_name = SO_LINGER;
  };
  struct  out_of_band_inline : public basic_option<out_of_band_inline, bool>
  {
  public:
    using basic_option::basic_option;
    using basic_option::operator=;
  private:
    friend basic_option<out_of_band_inline, bool>;
    inline static constexpr int static_level = SOL_SOCKET;
    inline static constexpr int static_name = SO_OOBINLINE;
  };
  struct  receive_buffer_size : public basic_option<receive_buffer_size, int>
  {
  public:
    using basic_option::basic_option;
    using basic_option::operator=;
  private:
    friend basic_option<receive_buffer_size, int>;
    inline static constexpr int static_level = SOL_SOCKET;
    inline static constexpr int static_name = SO_RCVBUF;
  };
  struct  receive_low_watermark : public basic_option<receive_low_watermark>
  {
  public:
    using basic_option::basic_option;
    using basic_option::operator=;
  private:
    friend basic_option<receive_low_watermark>;
    inline static constexpr int static_level = SOL_SOCKET;
    inline static constexpr int static_name = SO_RCVLOWAT;
  };
  struct  reuse_address : public basic_option<reuse_address, bool>
  {
  public:
    using basic_option::basic_option;
    using basic_option::operator=;
  private:
    friend basic_option<reuse_address, bool>;
    inline static constexpr int static_level = SOL_SOCKET;
    inline static constexpr int static_name = SO_REUSEADDR;
  };
  struct  send_buffer_size : public basic_option<send_buffer_size, int>
  {
  public:
    using basic_option::basic_option;
    using basic_option::operator=;
  private:
    friend basic_option<receive_buffer_size, int>;
    inline static constexpr int static_level = SOL_SOCKET;
    inline static constexpr int static_name = SO_SNDBUF;
  };
  struct  send_low_watermark : public basic_option<send_low_watermark>
  {
  public:
    using basic_option::basic_option;
    using basic_option::operator=;
  private:
    friend basic_option<send_low_watermark>;
    inline static constexpr int static_level = SOL_SOCKET;
    inline static constexpr int static_name = SO_SNDLOWAT;
  };
};
struct tcp_option
{
  struct no_delay : basic_option<no_delay, bool>
  {
    using basic_option::basic_option;
    using basic_option::operator=;
    static const int static_level = IPPROTO_TCP;
    static const int static_name = TCP_NODELAY;
  };
};
struct v6_only : public basic_option<v6_only, bool>
{
  using basic_option::basic_option;
  using basic_option::operator=;
private:
  friend basic_option<v6_only, bool>;
  static const int static_level = IPPROTO_IPV6;
  static const int static_name = IPV6_V6ONLY;
};
namespace unicast
{
/// Set the default number of hops (TTL) for outbound datagrams.
struct hops : public basic_option<hops>
{
  using basic_option::basic_option;
  using basic_option::operator=;
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  int level(auto const& protocol) const noexcept
  {
    return protocol.family() == AF_INET6 ? IPPROTO_IPV6 : IPPROTO_IP;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  int name(auto const& protocol) const noexcept
  {
    return protocol.family() == AF_INET6 ? IPV6_UNICAST_HOPS : IP_TTL;
  }
};
}
namespace multicast
{
struct multicast_option_base
{
  COASYNC_ATTRIBUTE((always_inline))
  explicit multicast_option_base(const address& group) noexcept
    : multicast_option_base(group.is_v4() ? multicast_option_base(group.to_v4()) : multicast_option_base(group.to_v6()))
  { }
  COASYNC_ATTRIBUTE((always_inline))
  explicit multicast_option_base(const address_v4& group, const address_v4& iface = address_v4::any()) noexcept
  {
    if constexpr (std::endian::native == std::endian::big)
      {
        _M_v4.imr_multiaddr.s_addr = group.to_uint();
        _M_v4.imr_interface.s_addr = iface.to_uint();
      }
    else if constexpr (std::endian::native == std::endian::little)
      {
        _M_v4.imr_multiaddr.s_addr = std::byteswap(group.to_uint());
        _M_v4.imr_interface.s_addr = std::byteswap(iface.to_uint());
      }
  }
  COASYNC_ATTRIBUTE((always_inline))
  explicit multicast_option_base(const address_v6& group, unsigned int iface = 0) noexcept
  {
    const auto bytes = group.to_bytes();
    std::memcpy(&_M_v6.ipv6mr_multiaddr.s6_addr, bytes.data(), 16);
    _M_v6.ipv6mr_interface = iface;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  int level(auto const& protocol) const noexcept
  {
    return protocol.family() == AF_INET6 ? IPPROTO_IPV6 : IPPROTO_IP;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  const char* data(auto const& protocol) const noexcept
  {
    return protocol.family() == AF_INET6 ? &_M_v6 : &_M_v4;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  std::size_t size(auto const& protocol) const noexcept
  {
    return protocol.family() == AF_INET6 ? sizeof(_M_v6) : sizeof(_M_v4);
  }
private:
  ipv6_mreq 	_M_v6 = {};
  ip_mreq 		_M_v4 = {};
};
/// Request that a socket joins a multicast group.
class join_group : private multicast_option_base
{
public:
  using multicast_option_base::multicast_option_base;
  using multicast_option_base::level;
  using multicast_option_base::data;
  using multicast_option_base::size;
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  int name(auto const& protocol) const noexcept
  {
    if (protocol.family() == AF_INET6)
      return IPV6_JOIN_GROUP;
    return IP_ADD_MEMBERSHIP;
  }
};
/// Request that a socket leaves a multicast group.
class leave_group : private multicast_option_base
{
public:
  using multicast_option_base::multicast_option_base;
  using multicast_option_base::level;
  using multicast_option_base::data;
  using multicast_option_base::size;
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  int name(auto const& protocol) const noexcept
  {
    if (protocol.family() == AF_INET6)
      return IPV6_LEAVE_GROUP;
    return IP_DROP_MEMBERSHIP;
  }
};
/// Specify the network interface for outgoing multicast datagrams.
class outbound_interface
{
public:
  COASYNC_ATTRIBUTE((always_inline))
  explicit outbound_interface(const address_v4& __v4) noexcept
  {
    if constexpr (std::endian::native == std::endian::big)
      _M_v4.s_addr = __v4.to_uint();
    else if constexpr(std::endian::native == std::endian::little)
      _M_v4.s_addr = std::byteswap(__v4.to_uint());
  }
  COASYNC_ATTRIBUTE((always_inline))
  explicit outbound_interface(unsigned int __v6) noexcept
    : _M_v4(), _M_v6(__v6)
  { }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  int level(auto const& protocol) const noexcept
  {
    return protocol.family() == AF_INET6 ? IPPROTO_IPV6 : IPPROTO_IP;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  int name(auto const& protocol) const noexcept
  {
    return protocol.family() == AF_INET6 ? IPV6_MULTICAST_IF : IP_MULTICAST_IF;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  const char* data(auto const& protocol) const noexcept
  {
    return protocol.family() == AF_INET6 ? &_M_v6 : &_M_v4;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  std::size_t size(auto const& protocol) const noexcept
  {
    return protocol.family() == AF_INET6 ? sizeof(_M_v6) : sizeof(_M_v4);
  }
private:
  in_addr 	_M_v4;
  unsigned 	_M_v6 = 0;
};
/// Set the default number of hops (TTL) for outbound datagrams.
class hops : public basic_option<hops>
{
public:
  using basic_option::basic_option;
  using basic_option::operator=;
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  int level(auto const& protocol) const noexcept
  {
    return protocol.family() == AF_INET6 ? IPPROTO_IPV6 : IPPROTO_IP;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  int name(auto const& protocol) const noexcept
  {
    return protocol.family() == AF_INET6 ? IPV6_MULTICAST_HOPS : IP_MULTICAST_TTL;
  }
};
/// Set whether datagrams are delivered back to the local application.
class enable_loopback : public basic_option<enable_loopback, bool>
{
public:
  using basic_option::basic_option;
  using basic_option::operator=;
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  int level(auto const& protocol) const noexcept
  {
    return protocol.family() == AF_INET6 ? IPPROTO_IPV6 : IPPROTO_IP;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  int name(auto const& protocol) const noexcept
  {
    return protocol.family() == AF_INET6 ? IPV6_MULTICAST_LOOP : IP_MULTICAST_LOOP;
  }
};
} // namespace multicast
}
}
#endif
