#ifndef COASYNC_MAGIC_GET_INCLUDED
#define COASYNC_MAGIC_GET_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../config.hpp"
#include <tuple>
/// boost::pfr::get ¡ª Returns reference or const reference to a field with
/// index I in simple aggregate val.
/// Recommended C++ Standards are C++17 and above. Library requires at least C++14!
/// Pre C++14 compilers (C++11, C++03...) are not supported.
/// magic_get function works with types that satisfy the requirements of SimpleAggregare:
/// aggregate types without base classes, const fields, references, or C arrays:

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
struct universal_type
{
  template <typename T>
  consteval operator T() const noexcept;
};

/// https://en.cppreference.com/w/cpp/language/structured_binding
/// Binds the specified names to subobjects or elements of the initializer.
/// Like a reference, a structured binding is an alias to an existing object.
/// Unlike a reference, a structured binding does not have to be of a reference type.
template <std::size_t> struct magic_get_impl;
#define magic_get_impl_xmacro(struct_arity, ...) \
template <> struct magic_get_impl<struct_arity> final {\
	template <std::size_t I, typename T>                  \
	COASYNC_ATTRIBUTE((nodiscard, always_inline)) constexpr static auto&& get(T& value) noexcept {\
		auto&& [__VA_ARGS__] = value;                                                    \
		return std::get<I>(std::forward_as_tuple(__VA_ARGS__));                          \
	}\
};

magic_get_impl_xmacro(1, _1);
magic_get_impl_xmacro(2, _1, _2);
magic_get_impl_xmacro(3, _1, _2, _3);
magic_get_impl_xmacro(4, _1, _2, _3, _4);
magic_get_impl_xmacro(5, _1, _2, _3, _4, _5);
#undef magic_get_impl_xmacro

template <typename T, typename... Args> requires std::is_aggregate_v<T>
COASYNC_ATTRIBUTE((nodiscard, always_inline)) consteval std::size_t struct_arity() noexcept
{
	static_assert(std::is_assignable_v<T, T>);
  if constexpr(std::is_empty_v<T>) return 0;
  if constexpr(not requires{T { std::declval<Args>() ...}; })
    return sizeof...(Args) - 1;
  else return struct_arity<T, Args ..., universal_type>();
}

// very basic reflection that gives you access to structure elements by index and
// provides other std::tuple like methods for user defined types without any
// macro or boilerplate code.

template <std::size_t struct_arity, std::size_t target, typename T> requires std::is_aggregate_v<T>
COASYNC_ATTRIBUTE((nodiscard, always_inline)) constexpr decltype(auto) magic_get(T& value) noexcept
{
  return magic_get_impl<struct_arity>::template get<target>(value);
}

}
}
#endif
