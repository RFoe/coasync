#ifndef COASYNC_REMOTE_QUEUE_INCLUDE
#define COASYNC_REMOTE_QUEUE_INCLUDE
#include "awaitable_frame_base.hpp"
#include "spin_loop_pause.hpp"
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
/// intrusive MPSC queue for remote_queue in task-stealing thread pool
/// Atomic operations are used for thread-safe and efficient handling of cases
/// where multiple producers add elements to the queue and single consumers
/// remove elements from the queue
template <auto Ptr> struct remote_queue;
template <typename Node, std::atomic<void*> Node::*_M_next>
struct remote_queue<_M_next>
{
private:
  std::atomic<void*> 	_M_back{&_M_nil};
  void* 							_M_front{&_M_nil};
  std::atomic<Node*> 	_M_nil = nullptr;
  void _M_push_back_nil()
  {
    _M_nil.store(nullptr, std::memory_order_relaxed);
    Node* prev = static_cast<Node*>(_M_back.exchange(&_M_nil, std::memory_order_acq_rel));
    (prev->*_M_next).store(&_M_nil, std::memory_order_release);
  }
public:
  COASYNC_ATTRIBUTE((maybe_unused)) bool push_back(Node* new_node) noexcept
  {
    (new_node->*_M_next).store(nullptr, std::memory_order_relaxed);
    void* prev_back = _M_back.exchange(new_node, std::memory_order_acq_rel);
    bool is_nil = prev_back == static_cast<void*>(&_M_nil);
    if (is_nil) COASYNC_ATTRIBUTE((likely))
        _M_nil.store(new_node, std::memory_order_release);
    else COASYNC_ATTRIBUTE((unlikely))
        (static_cast<Node*>(prev_back)->*_M_next).store(new_node, std::memory_order_release);
    return is_nil;
  }
  COASYNC_ATTRIBUTE((nodiscard)) Node* pop_front() noexcept
  {
    if (_M_front == static_cast<void*>(&_M_nil)) COASYNC_ATTRIBUTE((likely))
      {
        Node* next = _M_nil.load(std::memory_order_acquire);
        if (not next) COASYNC_ATTRIBUTE((likely)) return nullptr;
        _M_front = next;
      }
    Node* front = static_cast<Node*>(_M_front);
    void* next = (front->*_M_next).load(std::memory_order_acquire);
    if (next) COASYNC_ATTRIBUTE((unlikely))
      {
        _M_front = next;
        return front;
      }
    _M_push_back_nil();
    do
      {
				__spin_loop_pause();
        next = (front->*_M_next).load(std::memory_order_acquire);
      }
    while (!next);
    _M_front = next;
    return front;
  }
};
}
}
#endif
