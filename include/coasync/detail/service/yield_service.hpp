#ifndef COASYNC_YIELD_SERVICE_INCLUDED
#define COASYNC_YIELD_SERVICE_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../awaitable_frame_base.hpp"
#include "../basic_lockable.hpp"
#include <forward_list>
#include <mutex>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
/// do yield and releases the current coroutine's use of the thread's computing resources,
/// gives scheduling rights to the central scheduler, and resumes execution again
/// at some future time
template <typename execution_context>
struct basic_yield_service
{
  explicit basic_yield_service(execution_context& context) noexcept: _M_context(context) {}
  void post_frame(std::coroutine_handle<awaitable_frame_base> frame)
  {
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
		/// check concurrent threading
		/// Locking is not required for single thread
    _M_forward.emplace_front(frame);
  }
  void commit_frame()
  {
    std::forward_list<std::coroutine_handle<awaitable_frame_base>> outstanding;
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
		/// check concurrent threading
		/// Locking is not required for single thread
		_M_forward.swap(outstanding);
		/// swap out the coroutines to be resumed, present lock nesting[push_frame_to_executor does locking]
    if(alternative_lock.owns_lock()) COASYNC_ATTRIBUTE((likely)) alternative_lock.unlock();
    /// check concurrent threading
    for(std::coroutine_handle<awaitable_frame_base> frame: outstanding)
      _M_context.push_frame_to_executor(frame);
  }
private:
  execution_context&            _M_context;
  mutable basic_lockable 				_M_lockable;
  std::forward_list<std::coroutine_handle<awaitable_frame_base>>
      _M_forward;
};
typedef basic_yield_service<execution_context> yield_service;
}
}
#endif
