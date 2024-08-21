#ifndef __COASYNC_GET_FRAME_INCLUDED
#define __COASYNC_GET_FRAME_INCLUDED
#include "awaitable_frame.hpp"
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
/// Gets the handle behind the current coroutine[type erased]
struct get_frame
{
  bool await_ready() const noexcept
  {
    return false;
  }
  template <typename Ref, typename Alloc>
  std::coroutine_handle<>
  await_suspend(std::coroutine_handle<awaitable_frame<Ref, Alloc>> coroutine) const noexcept
  {
    _M_frame = std::coroutine_handle<awaitable_frame_base>::from_address(coroutine.address());
    return coroutine;
  }
  std::coroutine_handle<awaitable_frame_base> await_resume() const noexcept
  {
    return _M_frame;
  }
private:
  mutable std::coroutine_handle<awaitable_frame_base> _M_frame = nullptr;
};
}
}
#endif
