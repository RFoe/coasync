#ifndef __COASYNC_MESSAGE_FLAGS_INCLUDED
#define __COASYNC_MESSAGE_FLAGS_INCLUDED
#include "../detail/config.hpp"
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
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
/// Bitmask type for flags that can be passed to send and receive operations.
/// The flags argument is the bitwise OR of zero or more of the following flags.
enum class message_flags : int { };
constexpr message_flags message_peek = static_cast<message_flags>(MSG_PEEK);
constexpr message_flags message_out_of_band = static_cast<message_flags>(MSG_OOB);
constexpr message_flags message_do_not_route = static_cast<message_flags>(MSG_DONTROUTE);
COASYNC_ATTRIBUTE((nodiscard, always_inline))
constexpr message_flags operator&(message_flags f1, message_flags f2) noexcept
{
  return static_cast<message_flags>( static_cast<int>(f1) & static_cast<int>(f2) );
}
COASYNC_ATTRIBUTE((nodiscard, always_inline))
constexpr message_flags operator|(message_flags f1, message_flags f2) noexcept
{
  return static_cast<message_flags>( static_cast<int>(f1) | static_cast<int> (f2) );
}
COASYNC_ATTRIBUTE((nodiscard, always_inline))
constexpr message_flags operator^(message_flags f1, message_flags f2) noexcept
{
  return static_cast<message_flags>( static_cast<int>(f1) ^ static_cast<int>(f2) );
}
COASYNC_ATTRIBUTE((nodiscard, always_inline))
constexpr message_flags operator~(message_flags f) noexcept
{
  return static_cast<message_flags>( ~static_cast<int>(f) );
}
COASYNC_ATTRIBUTE((maybe_unused, always_inline))
constexpr message_flags& operator&=(message_flags& f1, message_flags f2) noexcept
{
  return f1 = (f1 & f2);
}
COASYNC_ATTRIBUTE((maybe_unused, always_inline))
constexpr message_flags& operator|=(message_flags& f1, message_flags f2) noexcept
{
  return f1 = (f1 | f2);
}
COASYNC_ATTRIBUTE((maybe_unused, always_inline))
constexpr message_flags& operator^=(message_flags& f1, message_flags f2) noexcept
{
  return f1 = (f1 ^ f2);
}
}
}
#endif
