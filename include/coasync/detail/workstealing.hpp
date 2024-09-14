#ifndef COASYNC_WORKSTEALING_INCLUDED
#define COASYNC_WORKSTEALING_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "awaitable_frame_base.hpp"
#include <vector>
#include <cassert>
#include <utility>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
/// A bleeding-edge lock-free, single-producer multi-consumer, work stealing deque
/// value type must be default initializable, trivially destructible and have nothrow move
/// constructor/assignment operators) and has no memory overhead associated with buffer recycling.
struct ring_buffer
{
  COASYNC_ATTRIBUTE((always_inline))
	explicit ring_buffer(std::int_least64_t capacity) noexcept
    : _M_capacity(capacity)
    , _M_mask(capacity - 1)
    , _M_storage(std::make_unique_for_overwrite<std::coroutine_handle<awaitable_frame_base>[]>(capacity))
  {
    assert(capacity >= 2 and (capacity & (capacity - 1)) == 0);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
	std::int_least64_t capacity() const noexcept
  {
    return _M_capacity;
  }
  COASYNC_ATTRIBUTE((always_inline))
	void store(std::int_least64_t inplace_index, std::coroutine_handle<awaitable_frame_base> frame) noexcept
  {
    _M_storage[inplace_index & _M_mask] = frame;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
	std::coroutine_handle<awaitable_frame_base> load(std::int_least64_t inplace_index) const noexcept
  {
    return _M_storage[inplace_index & _M_mask];
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
	ring_buffer* resize(std::int_least64_t assigned_bottom, std::int_least64_t assigned_top) const
  {
    ring_buffer* ptr = new ring_buffer { 2 * _M_capacity };
    for(std::int_least64_t index = assigned_top; index != assigned_bottom; index ++)
      ptr -> store(index, load(index));
    return ptr;
  }
private:
  std::int_least64_t _M_capacity;
  std::int_least64_t _M_mask;
  std::unique_ptr<std::coroutine_handle<awaitable_frame_base>[]> _M_storage;
};
/// Single producer multiple comsumer deque for task stealing
///
/// Lock-free single-producer multiple-consumer deque. Only the deque owner can perform pop and push
/// operations where the deque behaves like a stack. Others can (only) steal data from the deque, they see
/// a FIFO queue. All threads must have finished using the deque before it is destructed. T must be
/// default initializable, trivially destructible and have nothrow move constructor/assignment operators.
struct workstealing
{
  COASYNC_ATTRIBUTE((always_inline))
	explicit workstealing() noexcept
    : _M_top(0)
    , _M_bottom(0)
    , _M_buffer{new ring_buffer{1024}}
  {
    _M_garbage.reserve(32);
  }
  COASYNC_ATTRIBUTE((always_inline))
	~ workstealing()
  {
    delete _M_buffer.load(std::memory_order_relaxed);
  }
  void push(std::coroutine_handle<awaitable_frame_base> frame) noexcept
  {
    std::int_least64_t bottom = _M_bottom.load(std::memory_order_relaxed);
    std::int_least64_t top = _M_top.load(std::memory_order_acquire);
    ring_buffer* ringbuf = _M_buffer.load(std::memory_order_relaxed);
    if(ringbuf -> capacity() < (bottom - top) + 1) [[unlikely]]
      {
        _M_garbage.emplace_back(std::exchange(ringbuf, ringbuf -> resize(bottom, top)));
        _M_buffer.store(ringbuf, std::memory_order_relaxed);
      }
    ringbuf -> store(bottom, frame);
    std::atomic_thread_fence(std::memory_order_release);
    _M_bottom.store(bottom + 1, std::memory_order_relaxed);
  }
  COASYNC_ATTRIBUTE((nodiscard))
	std::coroutine_handle<awaitable_frame_base> try_pop() noexcept
  {
    std::int_least64_t bottom_edge = _M_bottom.load(std::memory_order_relaxed) - 1;
    ring_buffer* ringbuf = _M_buffer.load(std::memory_order_relaxed);
    _M_bottom.store(bottom_edge, std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_seq_cst);
    std::int_least64_t top = _M_top.load(std::memory_order_relaxed);
    if(top <= bottom_edge) [[likely]]
      {
        if(top == bottom_edge) [[unlikely]]
          {
            if(not _M_top.compare_exchange_strong(top, top + 1, std::memory_order_seq_cst, std::memory_order_relaxed)) [[likely]]
              {
                _M_bottom.store(bottom_edge + 1, std::memory_order_relaxed);
                return nullptr;
              }
            _M_bottom.store(bottom_edge + 1, std::memory_order_relaxed);
          }
        return ringbuf -> load(bottom_edge);
      }
    else
      {
        _M_bottom.store(bottom_edge + 1, std::memory_order_relaxed);
        return nullptr;
      }
  }
  COASYNC_ATTRIBUTE((nodiscard))
	std::coroutine_handle<awaitable_frame_base> try_steal() noexcept
  {
    std::int_least64_t top = _M_top.load(std::memory_order_acquire);
    std::atomic_thread_fence(std::memory_order_seq_cst);
    std::int_least64_t bottom = _M_bottom.load(std::memory_order_acquire);
    if(top < bottom) [[unlikely]]
      {
        std::coroutine_handle<awaitable_frame_base> frame = _M_buffer.load(std::memory_order_consume) -> load(top);
        if(not _M_top.compare_exchange_strong(top, top + 1, std::memory_order_seq_cst, std::memory_order_relaxed)) [[unlikely]]
          return nullptr;
        return frame;
      }
    else return nullptr;
  }
private:
  alignas(std::hardware_destructive_interference_size) std::atomic<std::int_least64_t> 	_M_top;
  alignas(std::hardware_destructive_interference_size) std::atomic<std::int_least64_t> 	_M_bottom;
  alignas(std::hardware_destructive_interference_size) std::atomic<ring_buffer*> 				_M_buffer;
  std::vector<std::unique_ptr<ring_buffer>> _M_garbage;
};
}
}
#endif
