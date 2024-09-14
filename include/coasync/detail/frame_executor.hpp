#ifndef COASYNC_FRAME_EXECUTOR_INCLUDED
#define COASYNC_FRAME_EXECUTOR_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "remote_queue.hpp"
#include "workstealing.hpp"
#include "basic_lockable.hpp"
#include "spin_loop_pause.hpp"
#include <thread>
#include <mutex>
#include <array>
#if defined(__cpp_lib_memory_resource)
# include <memory_resource>
#else
# error This library requires the use of C++17 pmr support
#endif

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
struct frame_executor
{
  COASYNC_ATTRIBUTE((always_inline)) explicit frame_executor(unsigned int concurrency) noexcept
    : _M_concurrency(concurrency)
    , _M_workstealing(concurrency)
    , _M_endpoint_mutex(concurrency)
    , _M_rotation(0)
  {
  }
  /// Submits a type erased coroutine handle to be resumed the remote queue
  /// for execution at a specific time in the future
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API push_frame(std::coroutine_handle<awaitable_frame_base> frame) noexcept
  {
    _M_remote_queue.push_back(remote_allocate(frame));
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  unsigned int COASYNC_API concurrency() const noexcept
  {
    return _M_concurrency;
  }
  /// Provides an interface for a specified producer thread (default main thread)
  /// to dispatch tasks from a remote queue to a local queue for individual child threads
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API dispatch_frame()
  {
    intrusive_value* ptr = nullptr;
    while((ptr = _M_remote_queue.pop_front()) != nullptr)
      {
        if(_M_concurrency) COASYNC_ATTRIBUTE((likely))  	push_local(ptr);
        /// If there are no other threads at this time, the coroutine can be resumed in place
        else COASYNC_ATTRIBUTE((unlikely))                resume_to(ptr -> _M_frame);
        remote_deallocate(ptr);
      }
  }
  /// launch the thread pool
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API request_launch()
  {
    if(_M_concurrency) COASYNC_ATTRIBUTE((likely))
      {
        for(unsigned int count {}; count < _M_concurrency; count ++)
          local_thread_invoke(count);
      }
  }
  /// cease the thread pool
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API request_stop()
  {
    if(_M_concurrency) COASYNC_ATTRIBUTE((likely))
      {
        for(auto& thread: _M_thread) thread.request_stop();
        for(auto& thread: _M_thread) thread.join();
        _M_thread.clear();
      }
  }
private:
  struct intrusive_value
  {
    std::coroutine_handle<awaitable_frame_base> _M_frame;
    std::atomic<void*>    											_M_next {};
  };
  void local_thread_invoke(unsigned int _S_local)
  {
    _M_thread.emplace_back([this, _S_local]
#if __cplusplus >= 202207L
                           COASYNC_ATTRIBUTE((nodiscard, always_inline))
#endif
                           (std::stop_token stoken) mutable
    {
      while(not stoken.stop_requested())
        {
/// In order to reduce the competition between worker threads and the common task queue,
/// let each worker thread hold a task queue, and the small tasks that are separated when
/// they do their own tasks are put into their own work queue. However, there is a problem
/// in this way, the initial tasks are large and small, some worker threads have finished
/// their own tasks, and other threads are still busy. This results in load imbalance.
/// In order to solve this problem, people have invented the work stealing algorithm,
/// the core of this algorithm[work stealing] is very simple, is the current worker
/// thread task queue is empty, go to the other worker thread task queue to get one (or more) tasks back
          if(auto frame = try_pop_local(_S_local)) COASYNC_ATTRIBUTE((unlikely))
            {
            	/// Attempts to fetch from the current worker thread's task queue
              resume_to(frame);
              detail::__spin_loop_pause();
            }
          else if(auto frame = try_steal_local(_S_local)) COASYNC_ATTRIBUTE((unlikely))
            {
            	/// attempts to steal tasks from another task queue
              resume_to(frame);
              detail::__spin_loop_pause();
            }
          else     COASYNC_ATTRIBUTE((likely))
            {
              std::this_thread::yield();
              detail::__spin_loop_pause();
            }
        }
    });
  }
  static void resume_to(std::coroutine_handle<awaitable_frame_base> frame)
  {
  	/// The coroutine handle may be automatically destroyed after it is resumed,
		/// so the semaphore associated with must be stored by reference in advance
    std::binary_semaphore& associated_semaphore = *frame.promise().get_semaphore();
    associated_semaphore.acquire();
    /// Ensure that the swapping out and swapping in of the coroutine stack have a one by one property
		/// to ensure thread safety
    frame.resume();
    /// Ensure that the swapping out and swapping in of the coroutine stack have a one by one property
		/// to ensure thread safety
    associated_semaphore.release();
  }
  COASYNC_ATTRIBUTE((nodiscard))
  intrusive_value* remote_allocate(std::coroutine_handle<awaitable_frame_base> frame)
  {
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_concurrency) COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_monotonic_lockable).swap(alternative_lock);
    /// check concurrency
    return std::pmr::polymorphic_allocator<std::byte> { &_M_monotonic_buffer }.new_object<intrusive_value>(frame);
  }
  void remote_deallocate(intrusive_value* ptr)
  {
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_concurrency) COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_monotonic_lockable).swap(alternative_lock);
    /// check concurrency
    std::pmr::polymorphic_allocator<std::byte> { &_M_monotonic_buffer }.delete_object(ptr);
  }
  void push_local(intrusive_value* ptr)
  {
    {
      std::lock_guard l { _M_endpoint_mutex[_M_rotation] };
      _M_workstealing[_M_rotation].push(ptr -> _M_frame);
    }
    /// use round robin algorithm to takes tasks from the remote queue and delivers
		/// them to each sub-thread
    _M_rotation = (_M_rotation + 1) % _M_concurrency;
  }
  COASYNC_ATTRIBUTE((nodiscard))
  std::coroutine_handle<awaitable_frame_base>
  try_pop_local(unsigned int local_inplace)
  {
    std::lock_guard l { _M_endpoint_mutex[local_inplace] };
    return _M_workstealing[local_inplace].try_pop();
  }
  COASYNC_ATTRIBUTE((nodiscard))
  std::coroutine_handle<awaitable_frame_base>
  try_steal_local(unsigned int local_inplace)
  {
  	/// Linearly traverse the local queue of other threads and attempt to steal tasks
    for(unsigned int stealing_victim {}; stealing_victim < _M_concurrency; stealing_victim ++)
      {
        if(stealing_victim == local_inplace) COASYNC_ATTRIBUTE((unlikely))
          continue;
        auto frame = _M_workstealing[stealing_victim].try_steal();
        if(frame) COASYNC_ATTRIBUTE((unlikely))
          return frame;
      }
    return nullptr;
  }
  unsigned int const 													_M_concurrency;
  remote_queue<&intrusive_value::_M_next> 		_M_remote_queue;
  std::vector<workstealing> 									_M_workstealing;
  std::vector<basic_lockable>             		_M_endpoint_mutex;
  unsigned int                            		_M_rotation;
  std::vector<std::jthread>              	 		_M_thread;
  COASYNC_ATTRIBUTE((maybe_unused)) std::array<std::byte, 256>
  _M_remote_buffer;
  basic_lockable                          		_M_monotonic_lockable;
  std::pmr::monotonic_buffer_resource 				_M_monotonic_buffer
  {
    &_M_remote_buffer, std::size(_M_remote_buffer), std::pmr::new_delete_resource()
  };
	/// a special-purpose memory resource class that releases the allocated memory
	/// only when the resource is destroyed. It is intended for very fast memory allocations
	/// in situations where memory is used to build up a few objects and then is released all at once.
};
}
}
#endif
