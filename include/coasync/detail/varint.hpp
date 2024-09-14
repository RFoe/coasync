#ifndef COASYNC_MAGIC_GET_INCLUDED
#define COASYNC_MAGIC_GET_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "config.hpp"
#include <cstdint>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
COASYNC_ATTRIBUTE((nodiscard, always_inline))
std::uint8_t* varint_encode(std::uint8_t* ptr, std::uint64_t val)
COASYNC_ATTRIBUTE((gnu::nonnull))
{
  while (val >= 0x80)
    {
      *ptr ++ = static_cast<std::uint8_t>(val) | 0x80;
      val >>= 7;
    }
  *ptr ++ = static_cast<std::uint8_t>(val);
  return ptr;
}
COASYNC_ATTRIBUTE((nodiscard, always_inline))
std::uint8_t* varint_decode(std::uint8_t* ptr, std::uint64_t* pVal)
 COASYNC_ATTRIBUTE((gnu::nonnull))
{
  std::uint8_t 	offset = 0;
  std::uint64_t result = 0;
  do
    {
      result |= static_cast<std::uint64_t>((*ptr) & ~0x80) << offset;
      offset += 7;
    }
  while (((*ptr ++) & 0x80) != 0ull);
  *pVal = result;
  return ptr;
}
COASYNC_ATTRIBUTE((nodiscard, always_inline))
std::uint8_t* zigzag_encode(std::uint8_t* ptr, std::int64_t val)
 COASYNC_ATTRIBUTE((gnu::nonnull))
{
  return varint_encode(ptr, static_cast<std::uint64_t>((val << 1) ^ (val >> 63)));
}
COASYNC_ATTRIBUTE((nodiscard, always_inline))
std::uint8_t* zigzag_decode(std::uint8_t* ptr, std::int64_t* pVal)
 COASYNC_ATTRIBUTE((gnu::nonnull))
{
  COASYNC_ATTRIBUTE((gnu::uninitialized)) uint64_t u64val;
  ptr = varint_decode(ptr, &u64val);
  *pVal = static_cast<std::int64_t>((u64val >> 1) ^ - (u64val & 1));
  return ptr;
}
}
}
#endif
