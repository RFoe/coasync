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
#include "when_any.hpp"
#include "when_all.hpp"

#include <optional>
#include <functional>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
inline constexpr detail::suspendible<detail::time_service> 	sleep_for;
inline constexpr detail::suspendible<detail::time_service> 	sleep_until;
inline constexpr detail::suspendible<detail::yield_service> yield;
}
template <typename RefX, typename AllocX, typename RefY, typename AllocY>
coasync::awaitable<
std::variant< coasync::detail::object_deduce_t<RefX>, coasync::detail::object_deduce_t<RefY>>,
    std::common_type_t<AllocX, AllocY>>
    COASYNC_API operator||(coasync::awaitable<RefX, AllocX> ax, coasync::awaitable<RefY, AllocY> ay)
{
  return coasync::when_any(std::move(ax), std::move(ay));
}
template <typename RefX, typename AllocX, typename RefY, typename AllocY>
coasync::awaitable<
std::tuple< coasync::detail::object_deduce_t<RefX>, coasync::detail::object_deduce_t<RefY>>,
    std::common_type_t<AllocX, AllocY>>
    COASYNC_API operator&&(coasync::awaitable<RefX, AllocX> ax, coasync::awaitable<RefY, AllocY> ay)
{
  return coasync::when_all(std::move(ax), std::move(ay));
}

template <typename Ref, typename Alloc, typename F>
requires (not std::is_void_v<Ref>)
and std::is_invocable_v<std::decay_t<F>, Ref>
and std::is_constructible_v<std::decay_t<F>, F&&>
coasync::awaitable<std::invoke_result_t<std::decay_t<F>, Ref>, Alloc>
COASYNC_API operator|(coasync::awaitable<Ref, Alloc> a, F&& f)
{
  using result_t = std::invoke_result_t<std::decay_t<F>, Ref>;
  return []
#if __cplusplus >= 202207L
         COASYNC_ATTRIBUTE((nodiscard))
#endif
         (coasync::awaitable<Ref, Alloc> __a, std::decay_t<F> __f)
         -> coasync::awaitable<result_t, Alloc>
  {
    co_return std::invoke(__f, static_cast<Ref&&>(co_await __a));
  }(std::move(a), std::forward<F>(f));
}
template <typename Alloc, typename F>
requires std::is_invocable_v<std::decay_t<F>>
    and std::is_constructible_v<std::decay_t<F>, F&&>
    coasync::awaitable<std::invoke_result_t<std::decay_t<F>>, Alloc>
    COASYNC_API operator|(coasync::awaitable<void, Alloc> a, F&& f)
{
  using result_t = std::invoke_result_t<std::decay_t<F>>;
  return []
#if __cplusplus >= 202207L
         COASYNC_ATTRIBUTE((nodiscard))
#endif
         (coasync::awaitable<void, Alloc> __a, std::decay_t<F> __f)
         -> coasync::awaitable<result_t, Alloc>
  {
    (void) (co_await __a);
    co_return std::invoke(__f);
  }(std::move(a), std::forward<F>(f));
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

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
template <typename Ref, typename Alloc, typename Rep, typename Period>
COASYNC_ATTRIBUTE((nodiscard))
awaitable<std::optional<detail::object_deduce_t<Ref>>>
COASYNC_API timeout(awaitable<Ref, Alloc> a, std::chrono::duration<Rep, Period> const& dura)
{
  execution_context& context = co_await detail::get_context();
  COASYNC_ATTRIBUTE((gnu::uninitialized)) long cancellation;
  auto result = co_await (std::move(a)
		|| detail::related_suspendible<detail::time_service>(context)(dura, std::addressof(cancellation)));
  if(result.index() == 1u) COASYNC_ATTRIBUTE((unlikely))
    co_return std::nullopt;
  try
    {
      use_service<detail::time_service>(context).cancel_frame(cancellation);
    }
  catch(coasync::cancellation_error& error)
    {
      if(error.code()
          != std::make_error_code(static_cast<std::errc>(cancellation_errc::no_frame_registered)))
        COASYNC_ATTRIBUTE((unlikely))
			std::rethrow_exception(std::make_exception_ptr(error));
      }
  co_return std::move(std::get<0>(result));
}
}

#endif
