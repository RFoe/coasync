#ifndef COASYNC_AWAITABLE_GROUP_INCLUDED
#define COASYNC_AWAITABLE_GROUP_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "awaitable.hpp"
#include <vector>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
template <typename Ref, typename Alloc = std::allocator<std::byte>>
using awaitable_group = std::vector<awaitable<Ref, Alloc>>;

template <typename T>
struct is_awaitable_group
	: public std::false_type {};
template <typename Ref, typename Alloc>
struct is_awaitable_group<awaitable_group<Ref, Alloc>>
	: public std::true_type {};
	
template <typename T>
inline constexpr bool is_awaitable_group_v
	= is_awaitable_group<T>::value;

}

#endif
