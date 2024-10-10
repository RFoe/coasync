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
/// a downward counter of type std::ptrdiff_t which can be used to synchronize coroutines.
/// The value of the counter is initialized on creation. Threads may block on the latch
/// until the counter is decremented to zero. There is no possibility to increase or reset
/// the counter, which makes the latch a single-use barrier.
template <typename execution_context>
struct [[nodiscard]] basic_co_latch
{
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit basic_co_latch(execution_context& context, std::ptrdiff_t count) noexcept
    : _M_mutex(context)
    , _M_condition(context)
    , _M_count(count) {};
  constexpr basic_co_latch& operator=(basic_co_latch const&) = delete;
  constexpr basic_co_latch(basic_co_latch const&) = delete;
  COASYNC_ATTRIBUTE((always_inline)) basic_co_latch(basic_co_latch&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) basic_co_latch& operator=(basic_co_latch&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) ~ basic_co_latch() noexcept = default;

  COASYNC_ATTRIBUTE((nodiscard)) awaitable<void> COASYNC_API count_down(std::ptrdiff_t update = 1) noexcept
  {
    auto __l  = co_await _M_mutex.scoped();
    COASYNC_ASSERT((_M_count >= update));
    _M_count -= update;
    if(_M_count == 0) COASYNC_ATTRIBUTE((unlikely))
      _M_condition.notify_all();
  }
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<bool> COASYNC_API try_wait() noexcept
  {
    auto __l  = co_await _M_mutex.scoped();
    co_return (_M_count == 0);
  }
  /// enables multiple tasks to synchronize the beginning of some computation.
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<void> COASYNC_API wait() noexcept
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
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<void> COASYNC_API arrive_and_wait(std::ptrdiff_t update = 1) noexcept
  {
    co_await count_down(update);
    co_await wait();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) execution_context& context() noexcept
  {
    return _M_mutex.context();
  }
private:
  mutable co_mutex 							_M_mutex;
  mutable co_condition_variable _M_condition;
  std::ptrdiff_t _M_count;
};
typedef basic_co_latch<execution_context> co_latch;
}
#endif
