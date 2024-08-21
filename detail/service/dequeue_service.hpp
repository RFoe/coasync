#ifndef COASYNC_DEQUEUE_SERVICE_INCLUDED
#define COASYNC_DEQUEUE_SERVICE_INCLUDED
#include "../awaitable_frame_base.hpp" 	/// for awaitable_frame_base
#include "../basic_lockable.hpp"        /// for basic_lockable
#include <forward_list>  	/// for forward_list
#include <mutex>          /// for unique_lock
#include <queue>          /// for queue
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
template <typename execution_context>
struct basic_dequeue_service
{
  struct composed_context
  {
    void* _M_queue;
    /// weak reference, non-owning
    void* _M_value;
    void* _M_mutex;
    /// use void* for type-erasing
    bool(* _M_delegate)(void*, void*, void*);
  };
  struct value_type
  {
    std::coroutine_handle<awaitable_frame_base> _M_frame;
    composed_context 														_M_context;
  };
  template <typename Value, typename Mutex>
  struct delegator
  {
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    static bool S_delegate(void* queue, void* value, void* mutex)
    {
      COASYNC_ATTRIBUTE((gnu::uninitialized)) bool queue_consumable;
      std::lock_guard l { * static_cast<Mutex*>(mutex) };
      /// Check whether data is available and ejected, and lock to prevent data race
      queue_consumable = not static_cast<std::queue<Value> *>(queue) -> empty();
      if(queue_consumable) COASYNC_ATTRIBUTE((unlikely))
        {
          ::new(value) Value { std::move(static_cast<std::queue<Value> *>(queue) -> front()) };
          static_cast<std::queue<Value> *>(queue) -> pop();
        }
      return queue_consumable;
    }
  };
  explicit basic_dequeue_service(execution_context& context) noexcept: _M_context(context) {}
  template <typename Value, typename Mutex>
  void post_frame(
    std::coroutine_handle<awaitable_frame_base> frame,
    /// post_frame overlaps
    std::queue<Value>& 	queue,
    Value& 							value,
    Mutex& 							mutex)
  {
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    /// check concurrent threading
    /// Locking is not required for single thread
    _M_forward.emplace_front(frame, composed_context
    {
      &queue, &value, &mutex, &delegator<Value, Mutex>::S_delegate
    });
  }
  void commit_frame()
  {
    std::forward_list<std::coroutine_handle<awaitable_frame_base>> outstanding;
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    /// test parallelism
    /// Locking is not required for single thread
    _M_forward.remove_if([this, &outstanding]
#if __cplusplus >= 202207L
                         COASYNC_ATTRIBUTE((nodiscard, always_inline))
#endif
                         (value_type& value) mutable -> bool
    {
      bool const queue_consumable
      = (* value._M_context._M_delegate)(
        value._M_context._M_queue,
        value._M_context._M_value,
        value._M_context._M_mutex);
      /// test and pop
      if(queue_consumable) COASYNC_ATTRIBUTE((unlikely))
        outstanding.emplace_front(value._M_frame);
      /// Save the coroutine handle to be resumed, and then execute it
      /// after unlocking the alternative_lock to prevent lock nesting
      return queue_consumable;
    });
    if(alternative_lock.owns_lock()) COASYNC_ATTRIBUTE((likely))
      alternative_lock.unlock();
    /// test parallelism
    for(std::coroutine_handle<awaitable_frame_base> frame: outstanding)
      _M_context.push_frame_to_executor(frame);
  }
private:
  execution_context& 						_M_context;
  mutable basic_lockable 				_M_lockable;
  std::forward_list<value_type> _M_forward;
};
typedef basic_dequeue_service<execution_context> dequeue_service;
}
}
#endif
