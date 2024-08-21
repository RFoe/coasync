#ifndef COASYNC_EXECUTION_CONTEXT_INCLUDED
#define COASYNC_EXECUTION_CONTEXT_INCLUDED
#include "detail/frame_executor.hpp"
#include "detail/frame_lifetime.hpp"
#include "detail/service_registry.hpp"
#include "detail/windows_initiate.hpp"
#include "detail/spin_loop_pause.hpp"
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
struct execution_context
{
  explicit execution_context(unsigned int concurrency = std::thread::hardware_concurrency()) noexcept
    :_M_executor(concurrency), _M_lifetime(static_cast<bool>(concurrency)) {}
  ~ execution_context()
  {
    erase_after(&_M_registry, *this);
  }
  /// Starts blocking polling and drives the event loop
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API loop()
  {
    /// start the thread pool(if concurreny > 0)
    _M_executor.request_launch();
    while(not _M_lifetime.empty())
      {
        /// Distribute tasks as sole producer
        _M_executor.dispatch_frame();
        std::this_thread::yield();
        detail::__spin_loop_pause();
        /// Query whether a schedulable coroutine has expired
        commit_after(&_M_registry, *this);
        detail::__spin_loop_pause();
        /// Destroy invalid coroutine groups
        _M_lifetime.remove_frame();
        detail::__spin_loop_pause();
      }
    _M_executor.request_stop();
  }
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API push_frame_to_executor(std::coroutine_handle<detail::awaitable_frame_base> frame)
  {
    /// post a task to executor/thread_pool
    _M_executor.push_frame(frame);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  unsigned int COASYNC_API concurrency() const noexcept
  {
    return _M_executor.concurrency();
  }
  template <typename Ref, typename Alloc>
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API  push_frame_to_lifetime(std::coroutine_handle<detail::awaitable_frame<Ref, Alloc>> frame)
  {
    /// Associate the coroutine object with this context
    frame.promise().set_context(this);
    frame.promise().set_id(_M_counter.fetch_add(1, std::memory_order_relaxed));
    /// Push it into the coroutine container, and monitor the life cycle
    _M_lifetime.push_frame(frame);
  }
  template <typename service_type>
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend service_type& COASYNC_API use_service(execution_context& context)
  {
    /// Linear query from beginning to end, if not created in place
    return find_from<service_type>(&context._M_registry, context);
  }
private:
  friend detail::service_registry;
  /// Provides memory resources for service registry to allocate schedulers
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  std::pmr::memory_resource* memory_resource() noexcept
  {
    return &_M_pool_resource;
  }
  detail::frame_executor   	_M_executor;
  detail::frame_lifetime   	_M_lifetime;
  detail::service_registry 	_M_registry
  {
    std::in_place_type<detail::null_service_t>
  };
  COASYNC_ATTRIBUTE((no_unique_address)) detail::windows_initiate
  _M_windows;
  std::atomic_long       		_M_counter;
  std::pmr::synchronized_pool_resource _M_pool_resource;
};
}
#endif
