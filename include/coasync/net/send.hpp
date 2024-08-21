#ifndef COASYNC_SEND_INCLUDED
#define COASYNC_SEND_INCLUDED
#include "../awaitable.hpp"
#include "option.hpp"
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
template <typename protocol_type, typename execution_context> struct __basic_socket;
struct tcp;
/// a composed asynchronous operation that writes a exact amount of data to a stream
/// before completion.
template <typename execution_context, typename Rng>
requires std::ranges::contiguous_range<Rng>
and std::ranges::sized_range<Rng>
and std::same_as<char, std::ranges::range_value_t<Rng>>
    COASYNC_ATTRIBUTE((nodiscard))
    awaitable<void> send_exactly(__basic_socket<tcp, execution_context>& socket, Rng const& buffer)
{
  std::size_t sentinel {};
  std::size_t transfer_bytes = std::ranges::size(buffer);
  while(sentinel < transfer_bytes)
    {
      int outcoming_bytes = co_await socket.send(std::ranges::subrange { buffer.begin() + sentinel, buffer.end() });
      if(outcoming_bytes <= 0) COASYNC_ATTRIBUTE((unlikely))
        throw std::system_error(static_cast<int>(std::errc::connection_aborted), std::generic_category());
      sentinel += outcoming_bytes;
    }
}
}
}
#endif
