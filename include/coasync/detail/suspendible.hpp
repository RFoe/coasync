#ifndef COASYNC_SUSPENDIBLE_INCULDED
#define COASYNC_SUSPENDIBLE_INCULDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../awaitable.hpp"
#include "get_frame.hpp"
#include "get_context.hpp"
#include "spin_loop_pause.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
/// cpo object: Different asynchronous operations are performed using different service types
/// The name cpo denotes a customization point object, which is a const function
/// object of a literal semiregular class type.
template <typename service_type>
struct [[nodiscard]] suspendible
{
  template <typename... Overlaps>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> operator()(Overlaps&& ... overlaps) const
  {
    std::coroutine_handle<awaitable_frame_base> frame = co_await get_frame();
    service_type& service = use_service<service_type>(co_await get_context());
    service.post_frame(frame, std::forward<Overlaps>(overlaps) ...);
    co_await std::suspend_always();
    detail::__spin_loop_pause();
  }
};

template <typename service_type>
struct [[nodiscard]] related_suspendible
{
  COASYNC_ATTRIBUTE((always_inline))
  explicit constexpr related_suspendible(execution_context& context) noexcept
    : _M_context(context) {}
  constexpr related_suspendible& operator=(related_suspendible const&) = delete;
  constexpr related_suspendible& operator=(related_suspendible&&) = delete;
  COASYNC_ATTRIBUTE((always_inline)) ~ related_suspendible() = default;
  template <typename... Overlaps>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> operator()(Overlaps&& ... overlaps) const&&
  {
    std::coroutine_handle<awaitable_frame_base> frame = co_await get_frame();
    use_service<service_type>(_M_context).post_frame(frame, std::forward<Overlaps>(overlaps) ...);
    co_await std::suspend_always();
    detail::__spin_loop_pause();
  }
private:
  execution_context& _M_context;
};
template <typename service_type, typename... Overlaps>
struct [[nodiscard]] transformed_awaitable
{
  COASYNC_ATTRIBUTE((always_inline))
  explicit constexpr transformed_awaitable(
    COASYNC_ATTRIBUTE((maybe_unused)) std::in_place_type_t<service_type>,
    Overlaps&& ... overlaps) noexcept
    : _M_overlaps(std::forward_as_tuple(static_cast<Overlaps&&>(overlaps) ...))
  {}
  constexpr transformed_awaitable& operator=(transformed_awaitable const&) = delete;
  constexpr transformed_awaitable& operator=(transformed_awaitable&&) = delete;
  COASYNC_ATTRIBUTE((always_inline)) ~ transformed_awaitable() = default;
  bool await_ready() const noexcept
  {
    return false;
  }
  template <typename Ref, typename Alloc>
  std::coroutine_handle<>
  await_suspend(std::coroutine_handle<awaitable_frame<Ref, Alloc>> coroutine) const noexcept
  {
    std::apply([coroutine](Overlaps&&... overlaps)
    {
    	execution_context& context = *coroutine.promise().get_context();
		  std::coroutine_handle<awaitable_frame_base> frame = std::coroutine_handle<awaitable_frame_base>::from_address(coroutine.address());
      use_service<service_type>(context).post_frame(frame, std::forward<Overlaps>(overlaps) ...);
    }, _M_overlaps);
    return std::noop_coroutine();
  }
  void await_resume() const noexcept
  {
  }
private:
  COASYNC_ATTRIBUTE((no_unique_address)) std::tuple<Overlaps&& ...> _M_overlaps;
};
#if defined(__cpp_deduction_guides)
template <typename service_type, typename... Overlaps>
transformed_awaitable(std::in_place_type_t<service_type>,Overlaps&& ...)
-> transformed_awaitable<service_type, Overlaps ...>;
#endif
}
}
#endif
