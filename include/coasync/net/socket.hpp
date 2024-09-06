#ifndef __COASYNC_SOCKET_INCLUDED
#define __COASYNC_SOCKET_INCLUDED
#include "../detail/suspendible.hpp"
#include "../detail/get_context.hpp"
#include "socket_base.hpp"
#include "option.hpp"
#include "message_flags.hpp"
#include <cstring>
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
struct tcp;
struct udp;
/// provides asynchronous and nonblocking stream/datagram-oriented socket functionality.
template <typename Range>
concept buffer_constrait = std::ranges::contiguous_range<Range>
///  providing a guarantee the denoted elements are stored contiguously in the memory.
                           and std::ranges::sized_range<Range>
///  specifies the requirements of a range type that knows its size in constant time with the size function.
                           and std::same_as<char, std::ranges::range_value_t<Range>>;
/// obtain the value type of the iterator type of range type R.
template <typename Protocol, typename execution_context>
struct __basic_socket: socket_base<execution_context>
{
  typedef socket_base<execution_context>::context_type 				context_type;
  typedef socket_base<execution_context>::native_handle_type 	native_handle_type;
  using socket_base<execution_context>::is_open;
  using socket_base<execution_context>::native_handle;
  using socket_base<execution_context>::context;
  using socket_base<execution_context>::cancel;
  using socket_base<execution_context>::close;
  using socket_base<execution_context>::non_blocking;
  using socket_base<execution_context>::io_control;
  using socket_base<execution_context>::available;
  using socket_base<execution_context>::listen;
  using socket_base<execution_context>::release;
  typedef Protocol 														protocol_type;
  typedef Protocol::endpoint 									endpoint_type;
  typedef Protocol::endpoint::address_type 		address_type;
  typedef Protocol::endpoint::address_v4_type address_v4_type;
  typedef Protocol::endpoint::address_v6_type address_v6_type;
  typedef Protocol::endpoint::port_type 			port_type;
  /// owns and manages a tcp/upd socket through native fd handle
	/// and disposes/closes of that socket when the unique_ptr goes out of scope.
  constexpr __basic_socket& operator=(__basic_socket const&) = delete;
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  protocol_type COASYNC_API protocol() const noexcept
  {
    return M_protocol;
  }
  /// sets the current value for a socket option associated with a socket of any type,
	/// in any state. Although options can exist at multiple protocol levels, they
	/// are always present at the uppermost socket level. Options affect socket operations,
	/// such as whether expedited data (OOB data for example) is received in the
	/// normal data stream, and whether broadcast messages can be sent on the socket.
  template <typename Option>
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API set_option(Option const& option) const
  {
    int retval = ::setsockopt(native_handle(),
                              option.level(protocol()),
                              option.name(protocol()),
                              option.data(protocol()),
                              option.size(protocol())
                             );
    if (retval == -1)   COASYNC_ATTRIBUTE((unlikely))
      throw std::system_error(detail::get_errno(),detail::generic_category());
  }
  /// retrieves the current value for a socket option associated with a socket
	/// of any type, in any state, and stores the result in optval. Options can
	/// exist at multiple protocol levels, but they are always present at the
	/// uppermost socket level. Options affect socket operations, such as the packet
	/// routing and OOB data transfer.
  template <typename Option>
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API get_option(Option& option) const
  {
    socklen_t option_length = option.size(protocol());
    int retval = ::getsockopt(native_handle(),
                              option.level(protocol()),
                              option.name(protocol()),
                              option.data(protocol()),
                              std::addressof(option_length)
                             );
    if (retval == -1)   COASYNC_ATTRIBUTE((unlikely))
      throw std::system_error(detail::get_errno(), detail::generic_category());
  }
  /// causes a socket descriptor and any related resources to be allocated and
	/// bound to a specific transport-service provider.
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API open(protocol_type const& proto = protocol_type::v4())
  {
    assert(not is_open());
    assign(proto, ::socket(
             static_cast<int>(proto.family()),
             static_cast<int>(proto.category()),
             static_cast<int>(proto.protocol())
           ));
    if (not is_open())   COASYNC_ATTRIBUTE((unlikely))
      throw std::system_error(detail::get_errno(), detail::generic_category());
  }
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API assign(protocol_type proto, native_handle_type sockfd) noexcept
  {
    M_protocol =	proto;
    M_sockfd =  	sockfd;
  }
  /// required on an unconnected socket before subsequent calls to the listen
	/// function. It is normally used to bind to either connection-oriented (stream)
	/// or connectionless (datagram) sockets. The bind function may also be used
	/// to bind to a raw socket (the socket was created by calling the socket
	/// function with the type parameter set to SOCK_RAW). The bind function may
	/// also be used on an unconnected socket before subsequent calls to the connect
	/// before send operation
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API bind(endpoint_type const& ep) const
  {
    assert(protocol().family() == ep.protocol().family());
    if (protocol() == protocol_type::v4())   COASYNC_ATTRIBUTE((likely))
      {
        ::sockaddr_in __sockaddr = ep.to_sockaddr_in();
        if (::bind(native_handle(),
                   reinterpret_cast<sockaddr* >(&__sockaddr),
                   sizeof(sockaddr_in)
                  ) == -1)   COASYNC_ATTRIBUTE((unlikely))
          throw std::system_error(detail::get_errno(), detail::generic_category());
      }
    else   COASYNC_ATTRIBUTE((unlikely))
      {
        ::sockaddr_in6 	__sockaddr6 = ep.to_sockaddr_in6();
        if (::bind(native_handle(),
                   reinterpret_cast<sockaddr* >(&__sockaddr6),
                   sizeof(sockaddr_in6)
                  ) == -1)   COASYNC_ATTRIBUTE((unlikely))
          throw std::system_error(detail::get_errno(), detail::generic_category());
      }
  }
  /// Get the local endpoint of the socket. This function is used to obtain the locally bound endpoint of the socket.
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  endpoint_type COASYNC_API local_endpoint() const
  {
    if(protocol() == protocol_type::v4())   COASYNC_ATTRIBUTE((likely))
      {
        ::sockaddr_in __sockaddr;
        ::socklen_t   __addrlen = sizeof(sockaddr_in);
        if (::getsockname(native_handle(),
                          reinterpret_cast<sockaddr* >(&__sockaddr),
                          std::addressof(__addrlen)
                         ) == native_handle_type(-1))   COASYNC_ATTRIBUTE((unlikely))
          throw std::system_error(detail::get_errno(), detail::generic_category());
        return endpoint_type::from_sockaddr_in(__sockaddr);
      }
    else   COASYNC_ATTRIBUTE((unlikely))
      {
        ::sockaddr_in6 __sockaddr6;
        ::socklen_t   __addrlen6 = sizeof(sockaddr_in6);
        if (::getsockname(native_handle(),
                          reinterpret_cast<sockaddr* >(&__sockaddr6),
                          std::addressof(__addrlen6)
                         ) == native_handle_type(-1))   COASYNC_ATTRIBUTE((unlikely))
          throw std::system_error(detail::get_errno(), detail::generic_category());
        return endpoint_type::from_sockaddr_in6(__sockaddr6);
      }
  }
  /// Get the remote endpoint of the socket. This function is used to obtain the remotely bound endpoint of the socket.
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  endpoint_type COASYNC_API remote_endpoint() const
  {
    if(protocol() == protocol_type::v4())   COASYNC_ATTRIBUTE((likely))
      {
        ::sockaddr_in __sockaddr;
        ::socklen_t   __addrlen = sizeof(sockaddr_in);
        if (::getpeername(native_handle(),
                          reinterpret_cast<sockaddr* >(&__sockaddr),
                          std::addressof(__addrlen)
                         ) == native_handle_type(-1))   COASYNC_ATTRIBUTE((unlikely))
          throw std::system_error(detail::get_errno(), detail::generic_category());
        return endpoint_type::from_sockaddr_in(__sockaddr);
      }
    else   COASYNC_ATTRIBUTE((unlikely))
      {
        ::sockaddr_in6 __sockaddr6;
        ::socklen_t   __addrlen6 = sizeof(sockaddr_in6);
        if (::getpeername(native_handle(),
                          reinterpret_cast<sockaddr* >(&__sockaddr6),
                          std::addressof(__addrlen6)
                         ) == native_handle_type(-1))   COASYNC_ATTRIBUTE((unlikely))
          throw std::system_error(detail::get_errno(), detail::generic_category());
        return endpoint_type::from_sockaddr_in6(__sockaddr6);
      }
  }
  /// create a connection to the specified destination. If socket s, is unbound,
	/// unique values are assigned to the local association by the system, and the
	/// socket is marked as bound. For connection-oriented sockets (for example,
	/// type SOCK_STREAM), an active connection is initiated to the foreign host
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API connect(endpoint_type const& ep)
  {
    assert(protocol().family() == ep.protocol().family());
    assert(&context() == &(co_await detail::get_context()));
    if(not is_open())   COASYNC_ATTRIBUTE((unlikely)) open();
    bool previous_non_blocking = non_blocking();
    non_blocking(true);
    ::sockaddr_in 	__sockaddr;
    ::sockaddr_in6 	__sockaddr6;
    if(protocol() == protocol_type::v4())   COASYNC_ATTRIBUTE((likely))
      __sockaddr = ep.to_sockaddr_in();
    else   COASYNC_ATTRIBUTE((unlikely))
      __sockaddr6 = ep.to_sockaddr_in6();
    ::sockaddr const* peer_addr = protocol() == protocol_type::v4()
                                  ? reinterpret_cast<sockaddr*>(&__sockaddr) : reinterpret_cast<sockaddr*>(&__sockaddr6);
    ::socklen_t const addr_len = protocol() == protocol_type::v4()
                                 ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
    int retval = ::connect(native_handle(), peer_addr, addr_len );
    if (retval >= 0)   COASYNC_ATTRIBUTE((unlikely))
      {
        non_blocking(previous_non_blocking);
        co_return;
      }
    if (detail::get_errno() != get_error_code(EINPROGRESS)
        and detail::get_errno() != get_error_code(EWOULDBLOCK)
        and detail::get_errno() != get_error_code(EINTR))   COASYNC_ATTRIBUTE((unlikely))
      throw std::system_error(detail::get_errno(),detail::generic_category());
    co_await detail::suspendible<detail::socketout_service>()(native_handle());
    socket_option::error err;
    get_option(err);
    if (int ec = err.get())   COASYNC_ATTRIBUTE((unlikely))
      throw std::system_error(ec, detail::generic_category());
  }
public:
	/// read incoming data on connection-oriented sockets, or connectionless sockets.
	/// When using a connection-oriented protocol, the sockets must be connected before
	/// calling receive.
  template <buffer_constrait Rng>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<int> COASYNC_API receive(Rng& buffer, message_flags flag = message_flags(0))
  requires std::same_as<protocol_type, tcp>
  {
    assert(&context() == &(co_await detail::get_context()));
    bool previous_non_blocking = non_blocking();
    non_blocking(true);
    while(true)
      {
        int transferred = ::recv(native_handle(), static_cast<char*>(std::ranges::data(buffer)),
                                 std::ranges::size(buffer), static_cast<int>(flag));
        if(transferred >= 0)   COASYNC_ATTRIBUTE((unlikely))
          {
            non_blocking(previous_non_blocking);
            co_return transferred;
          }
        if (detail::get_errno() != get_error_code(EINPROGRESS)
            and detail::get_errno() != get_error_code(EWOULDBLOCK)
            and detail::get_errno() != get_error_code(EINTR))   COASYNC_ATTRIBUTE((unlikely))
          throw std::system_error(detail::get_errno(), detail::generic_category());
        co_await detail::suspendible<detail::socketin_service>()(native_handle());
        socket_option::error err;
        get_option(err);
        if (int ec = err.get())   COASYNC_ATTRIBUTE((unlikely))
          throw std::system_error(ec, detail::generic_category());
      }
  }
  ///  to write outgoing data on a connected socket.
  template <buffer_constrait Rng>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<int> COASYNC_API send(Rng const& buffer, message_flags flag = message_flags(0))
  requires std::same_as<protocol_type, tcp>
  {
    assert(&context() == &(co_await detail::get_context()));
    bool previous_non_blocking = non_blocking();
    non_blocking(true);
    while(true)
      {
        int transferred = ::send(native_handle(), const_cast<char *>(std::ranges::data(buffer)),
                                 std::ranges::size(buffer), static_cast<int>(flag));
        if(transferred >= 0)   COASYNC_ATTRIBUTE((unlikely))
          {
            non_blocking(previous_non_blocking);
            co_return transferred;
          }
        if(detail::get_errno() != get_error_code(EINPROGRESS)
            and detail::get_errno() != get_error_code(EWOULDBLOCK)
            and detail::get_errno() != get_error_code(EINTR))  COASYNC_ATTRIBUTE((unlikely))
          throw std::system_error(detail::get_errno(),detail::generic_category());
        co_await detail::suspendible<detail::socketout_service>()(native_handle());
        socket_option::error err;
        get_option(err);
        if (int ec = err.get())   COASYNC_ATTRIBUTE((unlikely))
          throw std::system_error(ec, detail::generic_category());
      }
  }
  /// reads incoming data on both connected and unconnected sockets and captures the
	/// address from which the data was sent. This function is typically used with
	/// connectionless sockets. The local address of the socket must be known. For server
	/// applications, this is usually done explicitly through bind. Explicit binding
	/// is discouraged for client applications. For client applications using this
	/// function, the socket can become bound implicitly to a local address through send_to,
  template <buffer_constrait Rng>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<std::pair<int, endpoint_type>> COASYNC_API receive_from(Rng& buffer, int length, message_flags flag = message_flags(0))
                                        requires std::same_as<udp, protocol_type>
  {
    assert(&context() == &(co_await detail::get_context()));
    bool previous_non_blocking = non_blocking();
    non_blocking(true);
    ::sockaddr_in 		__sockaddr;
    ::sockaddr_in6 		__sockaddr6;
    while(true)
      {
        ::sockaddr const* to_sockaddr = protocol() == protocol_type::v4()
        ? reinterpret_cast<sockaddr*>(&__sockaddr) : reinterpret_cast<sockaddr*>(&__sockaddr6);
        ::socklen_t const to_socklen = protocol() == protocol_type::v4()
        ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
        int transferred = ::recvfrom( native_handle(), static_cast<char*>(std::ranges::data(buffer)),
                                      std::min(std::size_t(length), std::ranges::size(buffer)), static_cast<int>(flag), to_sockaddr, to_socklen);
        if (transferred == 0)   COASYNC_ATTRIBUTE((unlikely))
          {
            non_blocking(previous_non_blocking);
            co_return
            {
              transferred, protocol() == protocol_type::v4()
              ? endpoint_type::from_sockaddr_in(__sockaddr) : endpoint_type::from_sockaddr_in6(__sockaddr6)
            };
          }
        if (detail::get_errno() != get_error_code(EINPROGRESS)
            and detail::get_errno() != get_error_code(EWOULDBLOCK)
            and detail::get_errno() != get_error_code(EINTR))   COASYNC_ATTRIBUTE((unlikely))
          throw std::system_error(detail::get_errno(),detail::generic_category());
        co_await detail::suspendible<detail::socketout_service>()(native_handle());
        socket_option::error err;
        get_option(err);
        if (int ec = err.get())   COASYNC_ATTRIBUTE((unlikely))
          throw std::system_error(ec, detail::generic_category());
      }
  }
  ///  write outgoing data on a socket. For message-oriented sockets, care must be
	/// taken not to exceed the maximum packet size of the underlying subnets, which
	/// can be obtained by using get_option to retrieve the value of socket option
	/// max_message_size
  template <buffer_constrait Rng>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<int> COASYNC_API send_to(Rng& buffer, int length, endpoint_type const& ep, message_flags flag = message_flags(0))
  requires std::same_as<udp, protocol_type>
  {
    assert(protocol().family() == ep.protocol().family());
    assert(&context() == &(co_await detail::get_context()));
    bool previous_non_blocking = non_blocking();
    non_blocking(true);
    ::sockaddr_in 		__sockaddr;
    ::sockaddr_in6 		__sockaddr6;
    if(protocol() == protocol_type::v4())   COASYNC_ATTRIBUTE((likely))
      __sockaddr = ep.to_sockaddr_in();
    else   COASYNC_ATTRIBUTE((unlikely))
      __sockaddr6 = ep.to_sockaddr_in6();
    while(true)
      {
        ::sockaddr const* to_sockaddr = protocol() == protocol_type::v4()
        ? reinterpret_cast<sockaddr*>(&__sockaddr) : reinterpret_cast<sockaddr*>(&__sockaddr6);
        ::socklen_t const to_socklen = protocol() == protocol_type::v4()
        ? sizeof(sockaddr_in) : sizeof(sockaddr_in6);
        int transferred = ::sendto(native_handle(), static_cast<char*>(std::ranges::data(buffer)),
                                   std::min(std::size_t(length), std::ranges::size(buffer)), static_cast<int>(flag), to_sockaddr, to_socklen);
        if (transferred == 0)   COASYNC_ATTRIBUTE((unlikely))
          {
            non_blocking(previous_non_blocking);
            co_return transferred;
          }
        if (detail::get_errno() != get_error_code(EINPROGRESS)
            and detail::get_errno() != get_error_code(EWOULDBLOCK)
            and detail::get_errno() != get_error_code(EINTR))   COASYNC_ATTRIBUTE((unlikely))
          throw std::system_error(detail::get_errno(),detail::generic_category());
        co_await detail::suspendible<detail::socketout_service>()(native_handle());
        socket_option::error err;
        get_option(err);
        if (int ec = err.get())   COASYNC_ATTRIBUTE((unlikely))
          throw std::system_error(ec, detail::generic_category());
      }
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr __basic_socket(
    context_type& 				context,
    protocol_type const& 	protocol,
    native_handle_type 		sockfd = native_handle_type(-1))
  noexcept
    : socket_base<execution_context>(context, sockfd)
    , M_protocol(protocol)
  {
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr __basic_socket(__basic_socket&& other) noexcept
    : socket_base<execution_context>(std::move(other))
    , M_protocol(std::move(other.M_protocol))
  {
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr __basic_socket& operator=(__basic_socket&& other) noexcept
  {
    if (std::addressof(other) == this)   COASYNC_ATTRIBUTE((unlikely))
      return (*this);
    (socket_base<execution_context>&)*this = std::move(other);
    M_protocol = std::move(other.M_protocol);
    return (*this);
  }
  COASYNC_ATTRIBUTE((always_inline))
  ~ __basic_socket() noexcept
  {
    if (is_open())   COASYNC_ATTRIBUTE((unlikely))
      close();
  }
protected:
  using socket_base<execution_context>::M_context;
  protocol_type 												M_protocol;
  using socket_base<execution_context>::M_sockfd;
#if defined(_WIN32) || defined(_WIN64)
  using socket_base<execution_context>::M_bits;
#endif
  using socket_base<execution_context>::socket_base;
  using socket_base<execution_context>::operator=;
};
template <typename Protocol>
using basic_socket = __basic_socket<execution_context, Protocol>;
}
}
#endif
