#ifndef COASYNC_SERDE_STREAM_INCLUDED
#define COASYNC_SERDE_STREAM_INCLUDED
#include "../detail/meta/serde_stream_base.hpp"
#include "send.hpp"
#include "receive.hpp"
#include <sstream>
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
template <typename execution_context>
struct __serde_stream: detail::serde_stream_base<__serde_stream<execution_context>>
{
  using char_type = 			std::stringstream::char_type;
  using int_type = 				std::stringstream::int_type;
  using pos_type = 				std::stringstream::pos_type;
  using off_type = 				std::stringstream::off_type;
  using allocator_type = 	std::stringstream::allocator_type;
  using detail::serde_stream_base<__serde_stream>::serialize;
  using detail::serde_stream_base<__serde_stream>::deserialize;
  __serde_stream(__basic_socket<tcp, execution_context> socket) noexcept
    : _M_socket(std::move(socket)) {}
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> read(char_type* s, std::streamsize count)
  {
  	/// If there are not enough bytes of data in the current buffer, more data
		/// needs to be read from the socket.
    if(_M_available < count)
      {
        std::string buffer = co_await receive_at_least(_M_socket, count);
        _M_buffer.write(buffer.data(), buffer.size());
        _M_available += buffer.size();
      }
    _M_buffer.read(s, count);
    _M_available -= count;
  }
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> write(char_type const* s, std::streamsize count)
  {
    co_await send_exactly(_M_socket, std::span { s, static_cast<std::size_t>(count) });
  }
private:
  std::streamsize 												_M_available {0};
  std::stringstream 											_M_buffer;
  /// socket must be opened
  /// Using this socket for data transfers other than serialization is an unexpected behavior
  __basic_socket<tcp, execution_context> 	_M_socket;
};
typedef __serde_stream<execution_context> serde_stream;
}
}
#endif
