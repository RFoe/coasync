#ifndef COASYNC_FLAG_SERVICE_INCLUDED
#define COASYNC_FLAG_SERVICE_INCLUDED

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
template <typename execution_context>
struct basic_flag_service
{
private:
  struct value_type
  {
    std::coroutine_handle<awaitable_frame_base> _M_frame;
    std::atomic_flag* 													_M_flag;
    /// weak reference, non-owning
  };
public:
	typedef basic_lockable mutex_type;
	
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit basic_flag_service(execution_context& context) noexcept
    : _M_context(context) {}
  constexpr basic_flag_service& operator=(basic_flag_service const&) = delete;
  constexpr basic_flag_service(basic_flag_service const&) = delete;
  COASYNC_ATTRIBUTE((always_inline)) basic_flag_service(basic_flag_service&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) basic_flag_service& operator=(basic_flag_service&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) ~ basic_flag_service() noexcept = default;

  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  static constexpr std::size_t overlap_arity() noexcept
  {
    return 1u;
  }

  COASYNC_ATTRIBUTE((always_inline)) void post_frame(
    std::coroutine_handle<awaitable_frame_base> frame,
    /// post_frame: overlaps
    std::atomic_flag& flag)
  {
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    /// check parrallelism
    _M_forward.emplace_front(frame, &flag);
  }
  COASYNC_ATTRIBUTE((always_inline)) void commit_frame()
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
      bool const flag_set = value._M_flag -> test(std::memory_order_acquire);
      /// non-blocking test
      /// atomic_flag set: std::memory_order_release order required
      if(flag_set) COASYNC_ATTRIBUTE((unlikely)) outstanding.emplace_front(value._M_frame);
      /// Save the coroutine handle to be resumed, and then execute it
      /// after unlocking the alternative_lock to prevent lock nesting
      return flag_set;
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
typedef basic_flag_service<execution_context> flag_service;
}
}
#endif
