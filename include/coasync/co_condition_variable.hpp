#ifndef COASYNC_CO_CONDITION_VARIABLE_INCLUDED
#define COASYNC_CO_CONDITION_VARIABLE_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <thread> // for std::this_thread::yield()
#include <forward_list> // for std::forward_list

#include "co_mutex.hpp"
#include "detail/get_frame.hpp"
//#include "detail/get_context.hpp"
#include "detail/basic_lockable.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
struct execution_context; // forward declaration

/// a synchronization primitive used with a co_mutex to block one
/// or more coroutine until another coroutine both modifies a shared variable
/// (the condition) and notifies the co_condition_variable.
template <typename execution_context>
struct [[nodiscard]] basic_co_condition_variable
{
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit basic_co_condition_variable(execution_context& ctx) noexcept
    : _M_context(ctx)
  {};
  basic_co_condition_variable& operator=(basic_co_condition_variable const&) = delete;
  COASYNC_ATTRIBUTE((always_inline)) basic_co_condition_variable(basic_co_condition_variable&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) basic_co_condition_variable& operator=(basic_co_condition_variable&&) noexcept = default;
  ~ basic_co_condition_variable() noexcept = default;
  /// wait causes the current coroutine to block until the condition variable is notified
  /// or a spurious wakeup occurs. pred can be optionally provided to detect spurious wakeup.
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<void> COASYNC_API wait(co_mutex& mutex) const noexcept
  {
    std::coroutine_handle<detail::awaitable_frame_base> frame = co_await detail::get_frame();
    while(true)
      {
        mutex.unlock();
        COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<detail::basic_lockable>
        alternative_lock;
        if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
          std::unique_lock<detail::basic_lockable>(_M_lockable).swap(alternative_lock);
        _M_notify.emplace_front(frame);
        if(alternative_lock.owns_lock()) COASYNC_ATTRIBUTE((likely))
          alternative_lock.unlock();
        co_await std::suspend_always();
        co_await mutex.lock();
      }
  }
  template <typename Predicate> requires std::is_invocable_r_v<bool, Predicate>
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<void> COASYNC_API wait(co_mutex& mutex, Predicate&& pred) const noexcept
  {
    std::coroutine_handle<detail::awaitable_frame_base> frame = co_await detail::get_frame();
    while(not std::forward<Predicate>(pred).operator()())
      {
        mutex.unlock();
        COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<detail::basic_lockable>
        alternative_lock;
        if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
          std::unique_lock<detail::basic_lockable>(_M_lockable).swap(alternative_lock);
        _M_notify.emplace_front(frame);
        if(alternative_lock.owns_lock()) COASYNC_ATTRIBUTE((likely))
          alternative_lock.unlock();
        co_await std::suspend_always();
        co_await mutex.lock();
      }
    /// Right after wait returns, lock.owns_lock() is true, and lock.mutex() is
    /// locked by the calling thread. If these
    if(not std::forward<Predicate>(pred).operator()()) COASYNC_ATTRIBUTE((unlikely))
      COASYNC_TERMINATE();
  }
  ///provides a basic mechanism to notify a single task of an event.
  COASYNC_ATTRIBUTE((always_inline)) void COASYNC_API notify_all() const noexcept
  {
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<detail::basic_lockable> alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<detail::basic_lockable>(_M_lockable).swap(alternative_lock);
    if(not _M_notify.empty()) COASYNC_ATTRIBUTE((likely))
      {
        for(std::coroutine_handle<detail::awaitable_frame_base> frame: _M_notify)
          _M_context.push_frame_to_executor(frame);
        _M_notify.clear();
      }
    if(alternative_lock.owns_lock()) COASYNC_ATTRIBUTE((likely))
      alternative_lock.unlock();
    std::this_thread::yield();
  }
  /// The notifying coroutine does not need to hold the lock on the same mutex as the
  /// one held by the waiting thread(s). Doing so may be a pessimization, since
  /// the notified coroutine would immediately block again, waiting for the notifying
  /// coroutine to release the lock, though some implementations recognize the pattern
  /// and do not attempt to wake up the thread that is notified under lock.
  COASYNC_ATTRIBUTE((always_inline)) void COASYNC_API notify_one() const noexcept
  {
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<detail::basic_lockable>
    alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<detail::basic_lockable>(_M_lockable).swap(alternative_lock);
    /// waking a pending task if there is one.
    if(not _M_notify.empty()) COASYNC_ATTRIBUTE((likely))
      {
        _M_context.push_frame_to_executor(_M_notify.front());
        _M_notify.erase_after(_M_notify.before_begin());
      }
    if(alternative_lock.owns_lock()) COASYNC_ATTRIBUTE((likely))
      alternative_lock.unlock();
    std::this_thread::yield();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) execution_context& context() noexcept
  {
    return _M_context;
  }
private:
  COASYNC_ATTRIBUTE((no_unique_address)) execution_context& 							_M_context;
  mutable detail::basic_lockable 	_M_lockable;
  mutable std::forward_list<std::coroutine_handle<detail::awaitable_frame_base>> _M_notify;
};

typedef basic_co_condition_variable<execution_context> co_condition_variable;
}

#endif
