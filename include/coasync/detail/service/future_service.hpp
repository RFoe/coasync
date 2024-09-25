#ifndef COASYNC_FUTURE_SERVICE_INCLUDED
#define COASYNC_FUTURE_SERVICE_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../awaitable_frame_base.hpp"
#include "../basic_lockable.hpp"
#include <forward_list>
#include <future>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
template <typename execution_context>
struct basic_future_service
{
  struct composed_context
  {
    void* _M_future;
    /// use void* for type-erasing
    /// weak reference, non-owning
    bool(*_M_delegate)(void*);
  };
  template <typename T>
  struct delegator
  {
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    static bool S_delegate(void* future)
    {
      return static_cast<std::future<T>*>(future)->wait_for(std::chrono::nanoseconds(0)) == std::future_status::ready;
      /// Non-blocking wait, return immediately
    }
  };
  struct value_type
  {
    std::coroutine_handle<awaitable_frame_base> _M_frame;
    composed_context 														_M_context;
  };
  explicit basic_future_service(execution_context& context) noexcept: _M_context(context) {}
  template <typename T>
  void post_frame(
    std::coroutine_handle<awaitable_frame_base> frame,
    /// post_frame: overlaps
    std::future<T>& future)
  {
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    /// check parrallelism
    _M_forward.emplace_front(frame, composed_context { &future, &delegator<T>::S_delegate });
  }
  void commit_frame()
  {
    std::forward_list<std::coroutine_handle<awaitable_frame_base>> outstanding;
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    /// check parrallelism
    _M_forward.remove_if([this, &outstanding]
#if __cplusplus >= 202207L
                         COASYNC_ATTRIBUTE((nodiscard, always_inline))
#endif
                         (value_type& value) mutable -> bool
    {
      bool const future_gettable = (* value._M_context._M_delegate)(value._M_context._M_future);
      /// Once it is available, it is deleted in place, and wait is successfully triggered only once
      if(future_gettable) COASYNC_ATTRIBUTE((unlikely)) outstanding.emplace_front(value._M_frame);
      /// Save the coroutine handle to be resumed, and then execute it
      /// after unlocking the alternative_lock to prevent lock nesting
      return future_gettable;
    });
    if(alternative_lock.owns_lock()) COASYNC_ATTRIBUTE((likely)) alternative_lock.unlock();
    /// check parrallelism
    for(std::coroutine_handle<awaitable_frame_base> frame: outstanding)
      _M_context.push_frame_to_executor(frame);
  }
private:
  execution_context&            _M_context;
  mutable basic_lockable 				_M_lockable;
  std::forward_list<value_type> _M_forward;
};
typedef basic_future_service<execution_context> future_service;
}
}
#endif
