#ifndef COASYNC_ACCEPTOR_INCLUDED
#define COASYNC_ACCEPTOR_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "socket.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
/// Provides the ability to accept new connections.
template <typename Protocol, typename execution_context>
struct __basic_acceptor: public __basic_socket<Protocol, execution_context>
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
  using __basic_socket<Protocol, execution_context>::set_option;
  using __basic_socket<Protocol, execution_context>::get_option;
  using __basic_socket<Protocol, execution_context>::open;
  using __basic_socket<Protocol, execution_context>::assign;
  using __basic_socket<Protocol, execution_context>::bind;
  using __basic_socket<Protocol, execution_context>::local_endpoint;
  using __basic_socket<Protocol, execution_context>::remote_endpoint;
  using __basic_socket<Protocol, execution_context>::protocol;
  using __basic_socket<Protocol, execution_context>::send_to;
  using __basic_socket<Protocol, execution_context>::receive_from;
  using __basic_socket<Protocol, execution_context>::receive;
  using __basic_socket<Protocol, execution_context>::send;
  using __basic_socket<Protocol, execution_context>::connect;
  typedef Protocol 																	protocol_type;
  typedef Protocol::endpoint 												endpoint_type;
  typedef Protocol::endpoint::address_type 					address_type;
  typedef Protocol::endpoint::address_v4_type 			address_v4_type;
  typedef Protocol::endpoint::address_v6_type 			address_v6_type;
  typedef Protocol::endpoint::port_type 						port_type;
  using __basic_socket<Protocol, execution_context>::__basic_socket;
  using __basic_socket<Protocol, execution_context>::operator=;
  /// extracts the first connection on the queue of pending connections on current socket.
	/// It then creates and returns a handle to the new socket. The newly created socket
	/// is the socket that will handle the actual connection;
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<__basic_socket<protocol_type, context_type>> COASYNC_API accept()
  {
    assert(&context() == &(co_await detail::get_context()));
    co_await detail::suspendible<detail::socketin_service>()(native_handle());
    socket_option::error err;
    get_option(err);
    if (int ec = err.get()) COASYNC_ATTRIBUTE((unlikely))
      throw std::system_error(ec, detail::generic_category());
    native_handle_type sockfd = ::accept(native_handle(), nullptr, 0);
    if (sockfd == native_handle_type(-1)) COASYNC_ATTRIBUTE((unlikely))
      throw std::system_error(detail::get_errno(), detail::generic_category());
    co_return __basic_socket<protocol_type, context_type>(context(), protocol(), sockfd);
  }
  COASYNC_ATTRIBUTE((always_inline))
  __basic_acceptor(context_type& context, endpoint_type const& ep)
  noexcept
    : __basic_socket<Protocol, execution_context>(context, ep.protocol())
    , M_endpoint(ep)
  {
    open(ep.protocol());
    set_option(socket_option::reuse_address(true));
    bind(ep);
    listen(max_listen_connections);
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr __basic_acceptor(__basic_acceptor&& other) noexcept
    : __basic_socket<Protocol, execution_context>(std::move(other))
    , M_endpoint(std::move(other.M_endpoint))
  {
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr __basic_acceptor& operator=(__basic_acceptor&& other) noexcept
  {
    if (std::addressof(other) == this) COASYNC_ATTRIBUTE((unlikely))
      return (*this);
    static_cast<__basic_socket<Protocol, execution_context>&>(*this) = std::move(other);
    M_endpoint = std::move(other.M_endpoint);
    return (*this);
  }
  COASYNC_ATTRIBUTE((always_inline)) ~ __basic_acceptor() noexcept = default;
private:
  endpoint_type 		M_endpoint;
};
template <typename Protocol>
using basic_acceptor = __basic_acceptor<execution_context, Protocol>;
}
}
#endif
