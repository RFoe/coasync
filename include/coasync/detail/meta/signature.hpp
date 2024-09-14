#ifndef COASYNC_FUNCTION_TRAITS_INCLUDED
#define COASYNC_FUNCTION_TRAITS_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../config.hpp"
#if defined(__cpp_lib_source_location)
# include <source_location>
#else
# error This library requires the use of C++20 source_location support
#endif
#include <string_view>
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
/// Gets the type name at compile time
template <typename T>
COASYNC_ATTRIBUTE((nodiscard, always_inline))
consteval std::string_view signature() noexcept
{
/// https://en.cppreference.com/w/cpp/utility/source_location/function_name
/// std::source_location::function_name may help to obtain the names of functions
/// (including the special functions) alongside with their signatures.
  std::string_view signature {std::source_location::current().function_name()};
#if defined(__GNUC__) || defined(__clang__)
  std::size_t prefix_sentinel = std::min(signature.find_first_of("=") + 2u, signature.size());
  signature.remove_prefix(prefix_sentinel);
  std::size_t suffix_sentinel = signature.size() - signature.find_last_of(";");
  signature.remove_suffix(suffix_sentinel);
#elif defiend(_MSVC)
	std::size_t prefix_sentinel = signature.find_last_of("<") + 1;
	signature.remove_prefix(prefix_sentinel);
	std::size_t suffix_sentinel = signature.size() - signature.find_last_of(">");
	signature.remove_suffix(suffix_sentinel);
#endif
  return signature;
}
}
}
#endif
