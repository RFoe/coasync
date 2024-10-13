#ifndef COASYNC_DEADLINE_TIMER_INCLUDED
#define COASYNC_DEADLINE_TIMER_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "detail/suspendible.hpp"
#include "detail/service/time_service.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
enum class COASYNC_ATTRIBUTE((nodiscard)) deadline_timer_status: unsigned int
  {
  timeout = 0,
  cancelled
};

template <typename execution_context, typename Rep, typename Period>
struct COASYNC_ATTRIBUTE((nodiscard)) basic_deadline_timer
{
  typedef Rep rep;
  typedef Period period;

  template <typename OtherRep, typename OtherPeriod>
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit basic_deadline_timer(
    execution_context& ctx,
    std::chrono::duration<OtherRep, OtherPeriod> const& dura) noexcept
    : _M_context(ctx)
    , _M_duration(dura) {}

  constexpr basic_deadline_timer& operator=(basic_deadline_timer const&) = delete;
  constexpr basic_deadline_timer(basic_deadline_timer const&) = delete;
  COASYNC_ATTRIBUTE((always_inline)) basic_deadline_timer(basic_deadline_timer&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) basic_deadline_timer& operator=(basic_deadline_timer&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) ~ basic_deadline_timer() noexcept = default;

  COASYNC_ATTRIBUTE((nodiscard)) awaitable<deadline_timer_status> wait() const
  {
    try
      {
        co_await detail::related_suspendible<detail::time_service>(_M_context)(_M_duration, std::addressof(_M_cancellation));
      }
    catch(coasync::cancellation_error& error)
      {
        if(error.code() == std::make_error_code(static_cast<std::errc>(cancellation_errc::cancellation_requested))) COASYNC_ATTRIBUTE((likely))
          co_return deadline_timer_status::cancelled;
        std::rethrow_exception(std::make_exception_ptr(std::move(error)));
      }
    catch(...)
      {
        std::rethrow_exception(std::current_exception());
      }
    co_return deadline_timer_status::timeout;
  }
  COASYNC_ATTRIBUTE((always_inline)) void cancel() const noexcept(false)
  {
    try
      {
        use_service<detail::time_service>(_M_context).cancel_frame(_M_cancellation);
      }
    catch(coasync::cancellation_error& error)
      {
        if(error.code()
            == std::make_error_code(static_cast<std::errc>(cancellation_errc::no_frame_registered)))
          COASYNC_ATTRIBUTE((likely))
          return;
        std::rethrow_exception(std::make_exception_ptr(std::move(error)));
      }
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
