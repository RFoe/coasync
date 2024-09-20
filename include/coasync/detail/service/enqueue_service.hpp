#ifndef COASYNC_ENQUEUE_SERVICE_INCLUDED
#define COASYNC_ENQUEUE_SERVICE_INCLUDED
#include "../awaitable_frame_base.hpp"
#include "../basic_lockable.hpp"
#include <forward_list>
#include <mutex>
#include <queue>
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
template <typename execution_context>
struct basic_enqueue_service
{
  struct composed_context
  {
    void* 			_M_queue;
    /// weak reference, non-owning
    void* 			_M_value;
    void* 			_M_mutex;
    /// use void* for type-erasing
    std::size_t _M_bound;
    /// bounded channel, has capacity
    bool(* _M_delegate)(void*, void*, void*, std::size_t);
  };
  struct value_type
  {
    std::coroutine_handle<awaitable_frame_base> _M_frame;
    composed_context 														_M_context;
  };
  template <typename Value, typename Container, typename Mutex>
  struct delegator
  {
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    static bool S_delegate(void* queue, void* value, void* mutex, std::size_t bound)
    {
      COASYNC_ATTRIBUTE((gnu::uninitialized)) bool queue_producable;
      std::lock_guard l { * static_cast<Mutex*>(mutex) };
      /// Check whether data is out of bound and injected, and lock to prevent data race
      queue_producable = static_cast<std::queue<Value, Container> *>(queue) -> size() < bound;
      if(queue_producable) COASYNC_ATTRIBUTE((unlikely))
        static_cast<std::queue<Value, Container> *>(queue) -> emplace(std::move(*static_cast<Value*>(value)));
      return queue_producable;
    }
  };
  explicit basic_enqueue_service(execution_context& context) noexcept: _M_context(context) {}
  template <typename Value, typename Container, typename Mutex>
  void post_frame(
    std::coroutine_handle<awaitable_frame_base> frame,
    /// post_frame overlaps
    std::queue<Value, Container>& 	queue,
    Value& 							value,
    Mutex& 							mutex,
    std::size_t 				bound)
  {
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    /// check parrallelism
    _M_forward.emplace_front(frame, composed_context
    {
      &queue, &value, &mutex, bound, &delegator<Value, Container, Mutex>::S_delegate
    });
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
      bool const queue_producable
      = (* value._M_context._M_delegate)(
        value._M_context._M_queue,
        value._M_context._M_value,
        value._M_context._M_mutex,
        value._M_context._M_bound);
      /// test and push
      if(queue_producable) COASYNC_ATTRIBUTE((unlikely)) outstanding.emplace_front(value._M_frame);
      /// Save the coroutine handle to be resumed, and then execute it
      /// after unlocking the alternative_lock to prevent lock nesting
      return queue_producable;
    });
    if(alternative_lock.owns_lock()) COASYNC_ATTRIBUTE((likely)) alternative_lock.unlock();
    /// check parrallelism
    for(std::coroutine_handle<awaitable_frame_base> frame: outstanding)
      _M_context.push_frame_to_executor(frame);
  }
private:
  execution_context& 						_M_context;
  mutable basic_lockable 				_M_lockable;
  std::forward_list<value_type> _M_forward;
};
typedef basic_enqueue_service<execution_context> enqueue_service;
}
}
#endif
