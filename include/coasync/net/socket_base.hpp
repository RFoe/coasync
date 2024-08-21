#ifndef __COASYNC_SOCKET_BASE_INCLUDED
#define __COASYNC_SOCKET_BASE_INCLUDED
#include "../detail/networking.hpp"
#if defined(__has_include)
# if defined(__linux__)
#	 if __has_include(<unistd.h>)
#   include <unistd.h>
#  endif
#	 if __has_include(<sys/ioctl.h>)
#   include <sys/ioctl.h>
#  endif
#	 if __has_include(<sys/socket.h>)
#   include <sys/socket.h>
#  endif
#	 if __has_include(<sys/types.h>)
#   include <sys/types.h>
#  endif
# endif
#endif
#include "../detail/service/socket_service.hpp"
#include <utility>
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
/// The infrastructure service provider responsible for the socket sets the
/// backlog to the maximum reasonable value
inline constexpr int max_listen_connections = SOMAXCONN;
template <typename execution_context>
struct socket_base
{
  typedef detail::__native_handle 	native_handle_type;
  typedef execution_context 				context_type;
  /// a descriptor referencing the new socket.
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr native_handle_type COASYNC_API native_handle() const noexcept
  {
    return M_sockfd;
  }
  /// associated execution_context
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr context_type& COASYNC_API context() noexcept
  {
    return M_context;
  }
  /// has reference with a valid socket
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr bool COASYNC_API is_open() const noexcept
  {
    return native_handle() != native_handle_type(-1);
  }
	/// Cancel all asynchronous operations associated with the socket.
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API cancel()
  {
    if(not is_open())   COASYNC_ATTRIBUTE((unlikely))
		 return;
    use_service<detail::socketin_service>(context()).cancel_frame(native_handle());
    use_service<detail::socketout_service>(context()).cancel_frame(native_handle());
  }
  /// closes a socket. Use it to release the socket descriptor. Note that the socket
	/// descriptor may immediately be reused by the system as soon as closesocket
	/// function is issued.
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API close()
  {
    if (not is_open())   COASYNC_ATTRIBUTE((unlikely))
		 return;
    cancel();
#if defined(_WIN32) || defined(_WIN64)
    if (::closesocket(native_handle()) == -1)   COASYNC_ATTRIBUTE((unlikely))

#elif defined(__linux__)
    if (::close(native_handle()) == -1)   COASYNC_ATTRIBUTE((unlikely))

#endif
      throw std::system_error(detail::get_errno(),detail::generic_category());
    else   COASYNC_ATTRIBUTE((likely))
		 M_sockfd = native_handle_type(-1);
  }
	/// Sets the non-blocking mode of the socket.
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API non_blocking(bool mode)
  {
#if defined(_WIN32) || defined(_WIN64)
    int retval = ::ioctlsocket( native_handle(), FIONBIO, reinterpret_cast<unsigned long*>(&mode) );
    if (retval == -1)   COASYNC_ATTRIBUTE((unlikely))

      throw std::system_error(detail::get_errno(),detail::generic_category());
    M_bits.non_blocking = mode;
#elif defined(__linux__)
    int flag = ::fcntl(native_handle(), F_GETFL, 0);
    if (flag == -1)   COASYNC_ATTRIBUTE((unlikely))

      throw std::system_error(detail::get_errno(),detail::generic_category());
    if (mode)   COASYNC_ATTRIBUTE((likely))
		 flag |= O_NONBLOCK;
    else   COASYNC_ATTRIBUTE((unlikely))
		 flag &= ~O_NONBLOCK;
    flag = ::fcntl(native_handle(), F_SETFL, flag);
#endif
  }
  /// Gets the non-blocking mode of the socket.
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  bool COASYNC_API non_blocking() const
  {
#if defined(_WIN32) || defined(_WIN64)
    return M_bits.non_blocking;
#elif defined(__linux__)
    const int flags = ::fcntl(M_sockfd, F_GETFL, 0);
    if (flags == -1)   COASYNC_ATTRIBUTE((unlikely))

      throw std::system_error(detail::get_errno(),detail::generic_category());
    return flags & O_NONBLOCK;
#endif
  }
  /// used on any socket in any state. It is used to set or retrieve some operating parameters
	/// associated with the socket, independent of the protocol and communications subsystem.
  template<typename _Command, typename... _Args>
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API io_control(_Command&& __command, _Args&& ...__cmdargs) const
  {
#if defined(_WIN32) || defined(_WIN64)
    int retval = ::ioctlsocket(native_handle(), std::forward<_Command>,
                               std::forward<_Args>(__cmdargs) ...);
#elif defined(__linux__)
    int retval = ::ioctl( native_handle(), std::forward<_Command>(__command),
                          std::forward<_Args>(__cmdargs) ...);
#endif
    if (retval == -1)   COASYNC_ATTRIBUTE((unlikely))

      throw std::system_error(detail::get_errno(),detail::generic_category());
  }
  /// It tells how many bytes are in the buffer to read
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  std::size_t COASYNC_API available() const
  {
    if (not is_open())   COASYNC_ATTRIBUTE((unlikely))

      throw std::system_error(std::errc::bad_file_descriptor, std::generic_category());
#if defined(_WIN32) || defined(_WIN64)
    unsigned long avail = 0;
    if (::ioctlsocket(native_handle(), FIONREAD, &avail ) == -1)   COASYNC_ATTRIBUTE((unlikely))

      throw std::system_error(detail::get_errno(),detail::generic_category());
    return avail;
#elif defined(__linux__)
    int avail = 0;
    if (::ioctl( native_handle(), FIONREAD, &avail ) == -1)   COASYNC_ATTRIBUTE((unlikely))

      throw std::system_error(detail::get_errno(),detail::generic_category());
    return avail;
#endif
  }
  /// To accept connections, a socket is first created with the socket function and
	/// bound to a local address with the bind function. A backlog for incoming connections
	/// is specified with listen, and then the connections are accepted with the accept
	/// function. Sockets that are connection oriented, those of type SOCK_STREAM for example,
	/// are used with listen. The socket s is put into passive mode where incoming
	/// connection requests are acknowledged and queued pending acceptance by the process.
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API listen(int backlog ) const
  {
    if (::listen(native_handle(), backlog) == -1)   COASYNC_ATTRIBUTE((unlikely))

      throw std::system_error(detail::get_errno(),detail::generic_category());
  }
  /// Releases the ownership of the managed socket, if any.
	/// native_handle() returns -1 after the call.
	/// The caller is responsible for close the socket fd
  COASYNC_ATTRIBUTE((maybe_unused, always_inline))
  native_handle_type COASYNC_API release() noexcept
  {
    if (is_open())   COASYNC_ATTRIBUTE((likely))
		 cancel();
    return std::exchange(M_sockfd, native_handle_type(-1));
  }
protected:
  COASYNC_ATTRIBUTE((always_inline))
  constexpr socket_base(
    context_type& context,
    native_handle_type sockfd) noexcept
    : M_context(context)
    , M_sockfd(sockfd)
  {}
  COASYNC_ATTRIBUTE((always_inline))
  constexpr socket_base(socket_base&& other) noexcept
    : M_context(other.M_context)
    , M_sockfd(std::exchange(other.M_sockfd, native_handle_type(-1)))
#if defined(_WIN32) || defined(_WIN64)
    , M_bits(std::exchange(other.M_bits, {}))
#endif
  {
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr socket_base& operator=(socket_base&& other) noexcept
  {
    if (std::addressof(other) == this)   COASYNC_ATTRIBUTE((unlikely))
		 return (*this);
    M_context = other.M_context;
    M_sockfd = std::exchange(other.M_sockfd, native_handle_type(-1));
#if defined(_WIN32) || defined(_WIN64)
    M_bits = std::exchange(other.M_bits, {});
#endif
    return (*this);
  }
  context_type& M_context;
  native_handle_type M_sockfd;
#if defined(_WIN32) || defined(_WIN64)
  struct
  {
    unsigned int non_blocking: 1 				= 0;
    unsigned int connection_aborted: 1 	= 0;
  } M_bits;
#endif
};
}
}
#endif
