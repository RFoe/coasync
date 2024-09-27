#ifndef COASYNC_CO_LATCH_INCLUDED
#define COASYNC_CO_LATCH_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "co_mutex.hpp"
#include "co_condition_variable.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
struct execution_context;
template <typename execution_context>
struct [[nodiscard]] basic_co_latch
{
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit basic_co_latch(execution_context& context, std::ptrdiff_t count) noexcept
    : _M_condition(context)
    , _M_count(count) {};
  basic_co_latch& operator=(basic_co_latch const&) = delete;
  COASYNC_ATTRIBUTE((always_inline)) basic_co_latch(basic_co_latch&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) basic_co_latch& operator=(basic_co_latch&&) noexcept = default;
  ~ basic_co_latch() noexcept = default;
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<void> count_down(std::ptrdiff_t update = 1) noexcept
  {
    auto __l  = co_await _M_mutex.scoped();
    COASYNC_ASSERT((_M_count >= update));
    _M_count -= update;
    if(_M_count == 0) COASYNC_ATTRIBUTE((unlikely))
      _M_condition.notify_all();
  }
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<bool> try_wait() noexcept
  {
    auto __l  = co_await _M_mutex.scoped();
    co_return (_M_count == 0);
  }
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<void> wait() noexcept
  {
    auto __l  = co_await _M_mutex.scoped();
    co_await _M_condition.wait(_M_mutex, [this]
#if __cplusplus >= 202207L
                               COASYNC_ATTRIBUTE((nodiscard, always_inline))
#endif
                               () noexcept -> bool
    {
      return (_M_count == 0);
    });
  }
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<void> arrive_and_wait(std::ptrdiff_t update = 1) noexcept
  {
    co_await count_down(update);
    co_await wait();
  }
private:
  mutable co_mutex 							_M_mutex;
  mutable co_condition_variable _M_condition;
  std::ptrdiff_t _M_count;
};
typedef basic_co_latch<execution_context> co_latch;
}
#endif
