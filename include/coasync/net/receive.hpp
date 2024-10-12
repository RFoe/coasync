#ifndef COASYNC_RECEIVE_INCLUDED
#define COASYNC_RECEIVE_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../awaitable.hpp"
#include "option.hpp"
#include <algorithm>
#include <functional>

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
  COASYNC_ATTRIBUTE((gnu::uninitialized)) std::string result;
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
struct COASYNC_ATTRIBUTE((nodiscard)) receive_until_result
{
  std::string 									transferred;
  std::string::difference_type 	sentinel;
#if __cplusplus > 201703L && __cpp_impl_three_way_comparison >= 201907L
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) friend bool
  operator==(const receive_until_result&, const receive_until_result&) noexcept = default;
#endif
};
template<typename CharT, typename Traits>
COASYNC_ATTRIBUTE((maybe_unused, always_inline))
std::basic_ostream<CharT, Traits>& COASYNC_API operator<<(std::basic_ostream<CharT, Traits>& os, receive_until_result const& result) noexcept
{
  return (os << std::string_view{result.transferred.data(), std::size_t(result.sentinel)});
}
template <typename execution_context, typename BinaryPredicate = std::equal_to<char>>
COASYNC_ATTRIBUTE((nodiscard))
awaitable<receive_until_result> receive_until(__basic_socket<tcp, execution_context>& socket, std::string_view delimeter, BinaryPredicate predicate = BinaryPredicate())
{
  COASYNC_ATTRIBUTE((gnu::uninitialized)) socket_option::receive_buffer_size recvbuf;
  socket.get_option(recvbuf);
  COASYNC_ATTRIBUTE((gnu::uninitialized)) std::string 									result;
  COASYNC_ATTRIBUTE((gnu::uninitialized)) std::string 									fragment;
  static constexpr std::string::difference_type const sink_maximum = 1024u;
  fragment.resize(recvbuf.get());
  do
    {
      int incoming_bytes = co_await socket.receive(fragment);
      if(incoming_bytes == 0) COASYNC_ATTRIBUTE((unlikely))
        throw std::system_error(static_cast<int>(std::errc::connection_aborted), std::generic_category());
      if(result.length() >= std::string::size_type(sink_maximum)) COASYNC_ATTRIBUTE((unlikely))
        throw std::runtime_error("receive_until: sink overflow before delimeter is met");
      result.append(fragment, 0, incoming_bytes);
      auto sentinel_iteraotr
        = std::search(result.begin(), result.end(),
                      std::boyer_moore_horspool_searcher(
                        delimeter.begin(), delimeter.end(),
												std::hash<char>(), predicate));
      if(sentinel_iteraotr != result.end()) COASYNC_ATTRIBUTE((unlikely))
        {
          std::string::difference_type distance = std::distance(result.begin(), sentinel_iteraotr);
          co_return receive_until_result
          {
            std::move(result)
            , std::string::difference_type(distance + std::size(delimeter))
          };
        }
    }
  while(true);
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
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) std
{
template<> struct hash<coasync::net::receive_until_result> final
{
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  std::size_t operator()( coasync::net::receive_until_result const& result) const noexcept
  {
    return std::hash<std::string> {}(result.transferred) | std::hash<std::string::difference_type>{}(result.sentinel);
  }
};
template<>  struct formatter<coasync::net::receive_until_result>
{
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr auto parse(std::format_parse_context& context) const noexcept
  {
    return context.begin();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  auto format(coasync::net::receive_until_result const& result, std::format_context& context) const noexcept
  {
    return std::format_to(context.out(), "{}", std::string_view{result.transferred.data(), std::size_t(result.sentinel)});
  }
};
}
#endif
