#ifndef COASYNC_SERDE_STREAM_INCLUDED
#define COASYNC_SERDE_STREAM_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../detail/meta/serde_stream_base.hpp"
#include "socket.hpp"
#include "send.hpp"
#include "receive.hpp"
#include <sstream>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
template <typename execution_context>
struct basic_serde_stream: detail::serde_stream_base<basic_serde_stream<execution_context>>
{
  using char_type = 			std::stringstream::char_type;
  using int_type = 				std::stringstream::int_type;
  using pos_type = 				std::stringstream::pos_type;
  using off_type = 				std::stringstream::off_type;
  using allocator_type = 	std::stringstream::allocator_type;
  using detail::serde_stream_base<basic_serde_stream>::serialize;
  using detail::serde_stream_base<basic_serde_stream>::deserialize;

  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit basic_serde_stream(__basic_socket<tcp, execution_context> socket) noexcept
    : _M_socket(std::move(socket))
  {
    COASYNC_ASSERT((_M_socket.is_open()));
  }
  constexpr basic_serde_stream& operator=(basic_serde_stream const&) = delete;
  constexpr basic_serde_stream(basic_serde_stream const&) = delete;
  COASYNC_ATTRIBUTE((always_inline)) basic_serde_stream(basic_serde_stream&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) basic_serde_stream& operator=(basic_serde_stream&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) ~ basic_serde_stream() noexcept {
	};
  
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> read(char_type* s, std::streamsize count) COASYNC_ATTRIBUTE((gnu::nonnull))
  {
    /// If there are not enough bytes of data in the current buffer, more data
    /// needs to be read from the socket.
    if(_M_available < count) COASYNC_ATTRIBUTE((unlikely))
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
typedef basic_serde_stream<execution_context> serde_stream;
}
}
#endif
