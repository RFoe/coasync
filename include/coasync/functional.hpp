#ifndef COASYNC_FUNCTIONAL_INCLUDED
#define COASYNC_FUNCTIONAL_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "detail/service/time_service.hpp"
#include "detail/service/yield_service.hpp"
#include "detail/service/latch_service.hpp"
#include "detail/service/future_service.hpp"
#include "detail/service/flag_service.hpp"
#include "detail/suspendible.hpp"
#include "execution_context.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
inline constexpr detail::suspendible<detail::time_service> 	sleep_for;
inline constexpr detail::suspendible<detail::time_service> 	sleep_until;
inline constexpr detail::suspendible<detail::yield_service> yield;
}
template <typename Clock, typename Duration>
COASYNC_ATTRIBUTE((nodiscard, always_inline))
auto operator co_await(std::chrono::time_point<Clock, Duration> const& timeout) noexcept
{
	using in_place = std::in_place_type_t<coasync::detail::time_service>;
  return coasync::detail::transformed_awaitable{in_place(), timeout};
};
template <typename Rep, typename Period>
COASYNC_ATTRIBUTE((nodiscard, always_inline))
auto operator co_await(std::chrono::duration<Rep, Period> const& duration) noexcept
{
	using in_place = std::in_place_type_t<coasync::detail::time_service>;
  return coasync::detail::transformed_awaitable{in_place(), duration};
};
COASYNC_ATTRIBUTE((nodiscard, always_inline))
auto operator co_await(std::latch& lat) noexcept
{
	using in_place = std::in_place_type_t<coasync::detail::latch_service>;
  return coasync::detail::transformed_awaitable{in_place(), lat};
};
template <typename T>
COASYNC_ATTRIBUTE((nodiscard, always_inline))
auto operator co_await(std::future<T>& fut) noexcept
{
	using in_place = std::in_place_type_t<coasync::detail::future_service>;
  return coasync::detail::transformed_awaitable{in_place(), fut};
};
COASYNC_ATTRIBUTE((nodiscard, always_inline))
auto operator co_await(std::atomic_flag& flag) noexcept
{
	using in_place = std::in_place_type_t<coasync::detail::flag_service>;
  return coasync::detail::transformed_awaitable{in_place(), flag};
};

#endif
