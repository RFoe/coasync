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

template <typename execution_context>
struct [[nodiscard]] basic_co_condition_variable
{
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit basic_co_condition_variable(execution_context& ctx) noexcept
    : _M_context(ctx)
  {};
  basic_co_condition_variable& operator=(basic_co_condition_variable const&) = delete;
  COASYNC_ATTRIBUTE((always_inline)) basic_co_condition_variable(basic_co_condition_variable &&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) basic_co_condition_variable& operator=(basic_co_condition_variable &&) noexcept = default;
  ~ basic_co_condition_variable() noexcept = default;
  template <typename Predicate> requires std::is_invocable_r_v<bool, Predicate>
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<void> wait(co_mutex& mutex, Predicate&& pred) const noexcept
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
  }
  COASYNC_ATTRIBUTE((always_inline)) void notify_all() const noexcept
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
  COASYNC_ATTRIBUTE((always_inline)) void notify_one() const noexcept
  {
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<detail::basic_lockable>
    alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<detail::basic_lockable>(_M_lockable).swap(alternative_lock);
    if(not _M_notify.empty()) COASYNC_ATTRIBUTE((likely))
      {
        _M_context.push_frame_to_executor(_M_notify.front());
        _M_notify.erase_after(_M_notify.before_begin());
      }
    if(alternative_lock.owns_lock()) COASYNC_ATTRIBUTE((likely))
      alternative_lock.unlock();
    std::this_thread::yield();
  }
private:
  COASYNC_ATTRIBUTE((no_unique_address)) execution_context& 							_M_context;
  mutable detail::basic_lockable 	_M_lockable;
  mutable std::forward_list<std::coroutine_handle<detail::awaitable_frame_base>> _M_notify;
};

typedef basic_co_condition_variable<execution_context> co_condition_variable;
}

#endif
