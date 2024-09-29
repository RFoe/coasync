#ifndef COASYNC_CO_SEMAPHORE_INCLUDED
#define COASYNC_CO_SEMAPHORE_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "co_mutex.hpp"
#include "co_condition_variable.hpp"

#include <limits> /// for std::numeric_limits::max()

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
struct execution_context;
///a lightweight synchronization primitive that can control access to a shared resource.
/// Unlike a basic_co_mutex, a counting_semaphore allows more than one concurrent access
/// to the same resource, for at least LeastMaxValue concurrent accessors.
template <std::ptrdiff_t least_max_value, typename execution_context>
struct [[nodiscard]] basic_co_counting_semaphore
{
  static_assert(least_max_value <= std::numeric_limits<std::uint32_t>::max());
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit basic_co_counting_semaphore(execution_context& context, std::ptrdiff_t desired) noexcept
    : _M_condition(context)
    , _M_count(desired) {};
  basic_co_counting_semaphore& operator=(basic_co_counting_semaphore const&) = delete;
  COASYNC_ATTRIBUTE((always_inline)) basic_co_counting_semaphore(basic_co_counting_semaphore&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) basic_co_counting_semaphore& operator=(basic_co_counting_semaphore&&) noexcept = default;
  ~ basic_co_counting_semaphore() noexcept = default;
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) static constexpr COASYNC_API max() noexcept
  {
    return least_max_value;
  }
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<void> COASYNC_API acquire() noexcept
  {
    auto __l = co_await _M_mutex.scoped();
    co_await _M_condition.wait(_M_mutex, [this]
#if __cplusplus >= 202207L
                               COASYNC_ATTRIBUTE((nodiscard, always_inline))
#endif
                               () noexcept -> bool
    {
      return _M_count > 0;
    });
    -- _M_count;
  }
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<void> COASYNC_API release(std::ptrdiff_t update = 1) noexcept
  {
    COASYNC_ASSERT((update <= max() and update != 0));
    auto __l = co_await _M_mutex.scoped();
    COASYNC_ASSERT((_M_count <= max() - update));
    _M_count += update;
    if(update > 1) COASYNC_ATTRIBUTE((likely))
			_M_condition.notify_all();
		else COASYNC_ATTRIBUTE((unlikely))
			_M_condition.notify_one();
		co_return;
  }
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<bool> COASYNC_API try_release() noexcept
  {
		auto __l = co_await _M_mutex.scoped();
		if(_M_count != 0) COASYNC_ATTRIBUTE((unlikely)) {
			(void)(-- _M_count);
			co_return (true);
		}
		co_return (false);
	}
private:
  mutable co_mutex 							_M_mutex;
  mutable co_condition_variable _M_condition;
  std::ptrdiff_t 				_M_count;
};
template <std::ptrdiff_t least_max_value = std::numeric_limits<std::uint32_t>::max()>
using co_counting_semaphore = basic_co_counting_semaphore<least_max_value, execution_context>;

typedef basic_co_counting_semaphore<1ull, execution_context> co_binary_semaphore;

}

#endif
