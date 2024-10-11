#ifndef COASYNC_DEADLINE_TIMER_INCLUDED
#define COASYNC_DEADLINE_TIMER_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "detail/suspendible.hpp"
#include "detail/service/time_service.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
template <typename execution_context, typename Rep, typename Period>
struct COASYNC_ATTRIBUTE((nodiscard)) basic_deadline_timer
{
  typedef Rep rep;
  typedef Period period;

  template <typename OtherRep, typename OtherPeriod>
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit basic_deadline_timer(execution_context& ctx, std::chrono::duration<OtherRep, OtherPeriod> const& dura) noexcept
    : _M_context(ctx)
    , _M_duration(dura) {}

  constexpr basic_deadline_timer& operator=(basic_deadline_timer const&) = delete;
  constexpr basic_deadline_timer(basic_deadline_timer const&) = delete;
  COASYNC_ATTRIBUTE((always_inline)) basic_deadline_timer(basic_deadline_timer&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) basic_deadline_timer& operator=(basic_deadline_timer&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) ~ basic_deadline_timer() noexcept = default;

  COASYNC_ATTRIBUTE((nodiscard)) awaitable<void> wait() const
  {
    co_await detail::related_suspendible<detail::time_service>(_M_context)(_M_duration, std::addressof(_M_cancellation));
  }
  COASYNC_ATTRIBUTE((always_inline)) void cancel() const noexcept
  {
    use_service<detail::time_service>(_M_context).cancel_frame(_M_cancellation);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) execution_context& context() noexcept
  {
    return _M_context;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
	constexpr std::chrono::duration<Rep, Period> expire_at() const noexcept
  {
    return _M_duration;
  }
private:
  COASYNC_ATTRIBUTE((no_unique_address)) execution_context& _M_context;
  std::chrono::duration<Rep, Period> const _M_duration;
  mutable long _M_cancellation;
};
#if defined(__cpp_deduction_guides)
template <typename execution_context, typename OtherRep, typename OtherPeriod>
basic_deadline_timer(execution_context&, std::chrono::duration<OtherRep, OtherPeriod> const&)
-> basic_deadline_timer<execution_context, OtherRep, OtherPeriod>;
#endif
template <typename Rep = std::chrono::system_clock::rep, typename Period = std::ratio<1>>
using deadline_timer
  = basic_deadline_timer<execution_context, Rep, Period>;
}

#endif
