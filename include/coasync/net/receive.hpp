#ifndef COASYNC_RECEIVE_INCLUDED
#define COASYNC_RECEIVE_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../awaitable.hpp"
#include "option.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
template <typename protocol_type, typename execution_context> struct __basic_socket;
struct tcp;
/// a composed asynchronous operation that reads a exact amount of data from a stream
/// before completion.
template <typename execution_context>
COASYNC_ATTRIBUTE((nodiscard))
awaitable<std::string> receive_exactly(__basic_socket<tcp, execution_context>& socket, std::size_t transfer_bytes)
{
  std::string result;
  result.resize(transfer_bytes);
  int sentinel = 0;
  while(std::size_t(sentinel) < transfer_bytes)
    {
      auto receive_window = std::ranges::subrange {result.begin() + sentinel, result.end() };
      int incoming_bytes = co_await socket.receive(receive_window);
      if(incoming_bytes == 0) COASYNC_ATTRIBUTE((unlikely))
        throw std::system_error(static_cast<int>(std::errc::connection_aborted), std::generic_category());
      sentinel += incoming_bytes;
    }
  co_return result;
}
/// a composed asynchronous operation that reads data into a dynamic buffer sequence,
/// or into a streambuf, until it contains a delimiter, matches a regular expression,
/// or a function object indicates a match.
struct receive_until_result {
	std::string 									transferred;
	std::string::difference_type 	sentinel;
#if __cplusplus > 201703L && __cpp_impl_three_way_comparison >= 201907L
    COASYNC_ATTRIBUTE((nodiscard, always_inline)) friend bool
    operator==(const receive_until_result&, const receive_until_result&) noexcept = default;
#endif
};
template <typename execution_context>
COASYNC_ATTRIBUTE((nodiscard))
awaitable<receive_until_result> receive_until(__basic_socket<tcp, execution_context>& socket, std::string_view delimeter)
{
  COASYNC_ATTRIBUTE((gnu::uninitialized)) socket_option::receive_buffer_size recvbuf;
  socket.get_option(recvbuf);
  COASYNC_ATTRIBUTE((gnu::uninitialized)) std::string 									result;
  COASYNC_ATTRIBUTE((gnu::uninitialized)) std::string::difference_type 	sentinel;
  COASYNC_ATTRIBUTE((gnu::uninitialized)) std::string 									fragment;
  fragment.resize(recvbuf.get());
  do
    {
      int incoming_bytes = co_await socket.receive(fragment);
      if(incoming_bytes == 0) COASYNC_ATTRIBUTE((unlikely))
        throw std::system_error(static_cast<int>(std::errc::connection_aborted), std::generic_category());
      result.append(fragment, 0, incoming_bytes);
    }
  while((sentinel = result.rfind(delimeter)) == std::string::npos);
  co_return receive_until_result { std::move(result), sentinel };
}
/// a composed asynchronous operation that reads data into a dynamic buffer sequence,
/// or into a streambuf, it indicates that a read or write operation should continue until a minimum number
/// of bytes has been transferred, or until an error occurs.
template <typename execution_context>
COASYNC_ATTRIBUTE((nodiscard))
awaitable<std::string> receive_at_least(__basic_socket<tcp, execution_context>& socket, std::size_t transfer_bytes)
{
  COASYNC_ATTRIBUTE((gnu::uninitialized)) socket_option::receive_buffer_size recvbuf;
  socket.get_option(recvbuf);
  COASYNC_ATTRIBUTE((gnu::uninitialized)) std::string result;
  COASYNC_ATTRIBUTE((gnu::uninitialized)) std::string fragment;
  fragment.resize(recvbuf.get());
  while(result.size() < transfer_bytes)
    {
      int incoming_bytes = co_await socket.receive(fragment);
      if(incoming_bytes == 0) COASYNC_ATTRIBUTE((unlikely))
        throw std::system_error(static_cast<int>(std::errc::connection_aborted), std::generic_category());
      result.append(fragment, 0, incoming_bytes);
    }
  co_return result;
}
}
}
#endif
