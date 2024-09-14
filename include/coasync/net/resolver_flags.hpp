#ifndef COASYNC_RESOLVER_FLAGS_INCLUDED
#define COASYNC_RESOLVER_FLAGS_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

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

enum class resolver_flags : int { };
/// Flags that indicate options used in the getaddrinfo function.
/// Supported values for the ai_flags member can be a combination of the following options.
inline static constexpr resolver_flags passive						= static_cast<resolver_flags>(AI_PASSIVE);
inline static constexpr resolver_flags canonical_name			= static_cast<resolver_flags>(AI_CANONNAME);
inline static constexpr resolver_flags numeric_host				= static_cast<resolver_flags>(AI_NUMERICHOST);
inline static constexpr resolver_flags numeric_service		= static_cast<resolver_flags>(AI_NUMERICSERV);
inline static constexpr resolver_flags v4_mapped					= static_cast<resolver_flags>(AI_V4MAPPED);
inline static constexpr resolver_flags all_matching				= static_cast<resolver_flags>(AI_ALL);
inline static constexpr resolver_flags address_configured	= static_cast<resolver_flags>(AI_ADDRCONFIG);
COASYNC_ATTRIBUTE((nodiscard, always_inline))
constexpr resolver_flags operator&(resolver_flags f1, resolver_flags f2) noexcept
{
  return static_cast<resolver_flags>( static_cast<int>(f1) & int(f2) );
}
COASYNC_ATTRIBUTE((nodiscard, always_inline))
constexpr resolver_flags operator|(resolver_flags f1, resolver_flags f2) noexcept
{
  return static_cast<resolver_flags>( static_cast<int>(f1) | int(f2) );
}
COASYNC_ATTRIBUTE((nodiscard, always_inline))
constexpr resolver_flags operator^(resolver_flags f1, resolver_flags f2) noexcept
{
  return static_cast<resolver_flags>( static_cast<int>(f1) ^ static_cast<int>(f2) );
}
COASYNC_ATTRIBUTE((nodiscard, always_inline))
constexpr resolver_flags operator~(resolver_flags f) noexcept
{
  return static_cast<resolver_flags>( ~static_cast<int>(f) );
}
COASYNC_ATTRIBUTE((maybe_unused, always_inline))
constexpr resolver_flags& operator&=(resolver_flags& f1, resolver_flags f2) noexcept
{
  return f1 = (f1 & f2);
}
COASYNC_ATTRIBUTE((maybe_unused, always_inline))
constexpr resolver_flags& operator|=(resolver_flags& f1, resolver_flags f2) noexcept
{
  return f1 = (f1 | f2);
}
COASYNC_ATTRIBUTE((maybe_unused, always_inline))
constexpr resolver_flags& operator^=(resolver_flags& f1, resolver_flags f2) noexcept
{
  return f1 = (f1 ^ f2);
}
#endif
