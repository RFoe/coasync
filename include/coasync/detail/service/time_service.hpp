#ifndef COASYNC_TIME_SERVICE_INCLUDE
#define COASYNC_TIME_SERVICE_INCLUDE

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../awaitable_frame_base.hpp"
#include "../basic_lockable.hpp"
#include "../../cancellation_error.hpp"
#include <queue>
#include <forward_list>
#include <mutex>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
template <typename execution_context>
struct basic_time_service
{
private:
  struct value_type
  {
    std::chrono::system_clock::rep 	_M_rep;
    std::coroutine_handle<awaitable_frame_base> _M_frame;
    long 														_M_cancellation;
    /// cancellation token for deadline_timer
    COASYNC_ATTRIBUTE((always_inline))
    void request_cancel() noexcept
    {
      _M_cancellation = -1;
      /// Induce the asynchronous operation that has been initiated to return later
    }
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    bool cancel_requested() const noexcept
    {
      return _M_cancellation == -1;
      /// check cancelled
    }
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    bool operator<(value_type const& other) const noexcept
    {
      return _M_rep > other._M_rep;
    }
  };
  struct queue: std::priority_queue<value_type>
  {
    typedef std::priority_queue<value_type>::container_type::iterator iterator;
    /// default container: std::vector<value_type>
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    iterator begin()
    {
      return this->c.begin();
    }
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    iterator end()
    {
      return this->c.end();
    }
  };
public:
  typedef basic_lockable mutex_type;

  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit basic_time_service(execution_context& context) noexcept
    : _M_context(context) {}
  constexpr basic_time_service& operator=(basic_time_service const&) = delete;
  constexpr basic_time_service(basic_time_service const&) = delete;
  COASYNC_ATTRIBUTE((always_inline)) basic_time_service(basic_time_service&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) basic_time_service& operator=(basic_time_service&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) ~ basic_time_service() noexcept = default;

  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  static constexpr std::size_t overlap_arity() noexcept
  {
    return 2u;
  }
  template <typename Clock, typename Duration>
  COASYNC_ATTRIBUTE((always_inline)) void post_frame(
    std::coroutine_handle<awaitable_frame_base> 		frame,
    /// post_frame: overlaps
    std::chrono::time_point<Clock, Duration> const& timeout,
    /// optional cancellation token[sleep_for/until]
    long* 									cancellation = nullptr
  )
  {
    auto timeout_rep = timeout.time_since_epoch().count();
    if(cancellation) *cancellation = _M_counter.load(std::memory_order_acquire);
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_context.concurrency()) COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    /// check parrallelism
    _M_queue.emplace(timeout_rep, frame, _M_counter.fetch_add(1, std::memory_order_release));
  }
  template <typename Rep, typename Period>
  COASYNC_ATTRIBUTE((always_inline)) void post_frame(
    std::coroutine_handle<awaitable_frame_base> 	frame,
    std::chrono::duration<Rep, Period> const& 		duration,
    long* 									cancellation = nullptr)
  {
    /// reuse post_frame(..., std::chrono::time_point)
    post_frame(frame, std::chrono::system_clock::now() + duration, cancellation);
  }
  COASYNC_ATTRIBUTE((always_inline)) void commit_frame()
  {
    std::forward_list<std::coroutine_handle<awaitable_frame_base>> outstanding;
    auto present_rep = std::chrono::system_clock::now().time_since_epoch().count();
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_context.concurrency()) COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    /// check parrallelism
    while(not _M_queue.empty() and _M_queue.top()._M_rep < present_rep)
      {
        if(not _M_queue.top().cancel_requested()) COASYNC_ATTRIBUTE((likely))
          /// maybe cancelled: if so, will be ignored
          outstanding.emplace_front(_M_queue.top()._M_frame);
        /// Save the coroutine handle to be resumed, and then execute it
        /// after unlocking the alternative_lock to prevent lock nesting
        _M_queue.pop();
      }
    if(alternative_lock.owns_lock()) COASYNC_ATTRIBUTE((likely)) alternative_lock.unlock();
    /// check parrallelism
    for(std::coroutine_handle<awaitable_frame_base> frame: outstanding)
      _M_context.push_frame_to_executor(frame);
  }
  void COASYNC_API cancel_frame(long cancellation)
  {
    std::coroutine_handle<awaitable_frame_base> deprecated;
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    /// check parrallelism
    for(value_type& value: _M_queue)
      if(value._M_cancellation == cancellation) COASYNC_ATTRIBUTE((unlikely))
        {
          value.request_cancel();
          deprecated = value._M_frame;
          break;
        }
    if(alternative_lock.owns_lock()) COASYNC_ATTRIBUTE((likely)) alternative_lock.unlock();
    /// check parrallelism
    if(deprecated != nullptr) COASYNC_ATTRIBUTE((likely))
      {
      	std::exception_ptr eptr = std::make_exception_ptr(cancellation_error(cancellation_errc::cancellation_requested));
				deprecated.promise().set_exception(std::move(eptr));
        _M_context.push_frame_to_executor(deprecated);
      }
    else  COASYNC_ATTRIBUTE((unlikely))
			throw cancellation_error(cancellation_errc::no_frame_registered);
    /// resume immediately with cancellation error
  }
private:
  execution_context& 			_M_context;
  mutable basic_lockable 	_M_lockable;
  queue           				_M_queue;
  std::atomic_long      	_M_counter;
};
typedef basic_time_service<execution_context> time_service;
}
}
#endif
