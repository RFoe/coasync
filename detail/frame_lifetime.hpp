#ifndef COASYNC_FRAME_LIFETIME_INCLUDED
#define COASYNC_FRAME_LIFETIME_INCLUDED
#include "frame_delegate.hpp"
#include "basic_lockable.hpp"
#include <forward_list>
#include <mutex>
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
/// Coroutine container, used to check the termination state of the coroutine in polling
/// and periodically clean up the coroutine
struct frame_lifetime
{
private:
  struct value_type
  {
    value_type(std::coroutine_handle<awaitable_frame_base> frame, frame_delegate::delegate_type fp) noexcept
      : _M_frame(frame)
      , _M_delegate(fp)
        /// The semaphore is initialized to the available state
      , _M_semaphore(1)
    {}
    COASYNC_ATTRIBUTE((nodiscard)) static bool destructible(value_type& value)
    {
      COASYNC_ATTRIBUTE((gnu::uninitialized)) bool frame_complete;
      COASYNC_ATTRIBUTE((assume(!!value._M_delegate)));
      (* value._M_delegate)(frame_delegate::Op_done, value._M_frame, &frame_complete);
      if(frame_complete) COASYNC_ATTRIBUTE((unlikely))
        {
          value._M_semaphore.acquire();
          /// The executor will do the release operation after the coroutine completes,
          /// so the container needs to do a acquire operation before destroying the semaphore
          (* value._M_delegate)(frame_delegate::Op_destroy, value._M_frame, nullptr);
        }
      return frame_complete;
    }
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
		std::binary_semaphore& get_semaphore() noexcept
    {
      return _M_semaphore;
    }
  private:
    std::coroutine_handle<awaitable_frame_base>
    _M_frame;
    frame_delegate::delegate_type _M_delegate;
    std::binary_semaphore   			_M_semaphore;
  };
public:
  explicit frame_lifetime(bool concurrent_hint) noexcept: _M_concurrent_hint(concurrent_hint) {}
  /// Pushes a coroutine handle and manages the life cycle
  template <typename Ref, typename Alloc>
	COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API push_frame(std::coroutine_handle<awaitable_frame<Ref, Alloc>> frame)
  {
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_concurrent_hint) COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    /// check concurrency
    frame.promise().set_semaphore(
      &_M_frames.emplace_front(
        /// Do type erasing and save them in a unified manner
        std::coroutine_handle<detail::awaitable_frame_base>::from_address(frame.address()),
        frame_delegate::delegator<Ref, Alloc>::S_delegate
      ).get_semaphore());
  }
	COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API remove_frame()
  {
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_concurrent_hint) COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    /// check concurrency
    _M_frames.remove_if(&value_type::destructible);
  }
  /// As a sign to end polling
	COASYNC_ATTRIBUTE((nodiscard, always_inline))
  bool COASYNC_API empty() const
  {
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_concurrent_hint) COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    /// check concurrency
    return _M_frames.empty();
  }
private:
  std::forward_list<value_type> _M_frames;
  mutable basic_lockable 				_M_lockable;
  bool const 										_M_concurrent_hint;
};
}
}
#endif
