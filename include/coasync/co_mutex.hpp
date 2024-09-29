#ifndef COASYNC_CO_MUTEX_INCLUDED
#define COASYNC_CO_MUTEX_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "awaitable.hpp"
#include "detail/service/yield_service.hpp"
#include "detail/suspendible.hpp"

#if __cpp_impl_coroutine >= 201902 && __cpp_lib_coroutine >= 201902
#  include <coroutine>
#elif defined(__cpp_coroutines) && __has_include(<experimental/coroutine>)
#  include <experimental/coroutine>
namespace std
{
using std::experimental::coroutine_handle;
using std::experimental::coroutine_traits;
using std::experimental::noop_coroutine;
using std::experimental::suspend_always;
using std::experimental::suspend_never;
} // namespace std
#else
# error This library requires the use of C++20 coroutine support
#endif

#include <atomic> // for std::atomic_bool
#include <cstdint> // for std::intptr_t
#include <mutex> // for std::unique_lock

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
struct execution_context;

/// Contrary to popular belief, it is ok and often preferred to use the ordinary
/// Mutex from the standard library in asynchronous code.

/// The feature that the async mutex offers over the blocking mutex is the ability
/// to keep it locked across an .await point. This makes the async mutex more
/// expensive than the blocking mutex, so the blocking mutex should be preferred
/// in the cases where it can be used.

template <typename execution_context>
struct [[nodiscard]] basic_co_mutex
{
  COASYNC_ATTRIBUTE((always_inline)) constexpr explicit basic_co_mutex() noexcept = default;
  basic_co_mutex& operator=(basic_co_mutex const&) = delete;
  COASYNC_ATTRIBUTE((always_inline)) basic_co_mutex(basic_co_mutex&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) basic_co_mutex& operator=(basic_co_mutex&&) noexcept = default;
  ~ basic_co_mutex() noexcept = default;
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) bool try_lock() const noexcept
  {
    return not _M_locked.exchange(true, std::memory_order_acquire);
  }
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<void> COASYNC_API lock() const noexcept
  {
    std::intptr_t cnt = _S_count;
    while(not try_lock())
      while(_M_locked.load(std::memory_order_relaxed))
        if(cnt -- <= 0) COASYNC_ATTRIBUTE((unlikely))
          {
/// The locking mechanism uses eventual fairness to ensure locking
/// will be fair on average without sacrificing performance.
            co_await detail::suspendible<detail::yield_service>()();
            cnt = _S_count;
          }
    co_return;
  }
  COASYNC_ATTRIBUTE((always_inline)) void COASYNC_API unlock() const noexcept
  {
    _M_locked.store(false, std::memory_order_release);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) bool COASYNC_API owns_lock() const noexcept
  {
    return _M_locked.load(std::memory_order_acquire);
  }
  /// An RAII implementation of a ¡°scoped lock¡± of a mutex.
	/// When this structure is dropped (falls out of scope), the lock will be unlocked.
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<std::unique_lock<basic_co_mutex const>> COASYNC_API scoped() const noexcept
  {
    co_await this->lock();
    co_return std::unique_lock(* this, std::adopt_lock);
  }
private:
  mutable std::atomic_bool 				_M_locked {false};
  static constexpr std::intptr_t const _S_count = 1024;
};
typedef basic_co_mutex<execution_context> co_mutex;

}

#endif
