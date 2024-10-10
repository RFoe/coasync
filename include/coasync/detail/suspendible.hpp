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
}
}
#endif
