#ifndef __COASYNC_GET_ID_INCLUDED
#define __COASYNC_GET_ID_INCLUDED
#include "awaitable_frame.hpp"
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
/// Gets the current coroutine id
struct get_id
{
  bool await_ready() const noexcept
  {
    return false;
  }
  template <typename Ref, typename Alloc>
  std::coroutine_handle<>
  await_suspend(std::coroutine_handle<awaitable_frame<Ref, Alloc>> coroutine) const noexcept
  {
    _M_id = coroutine.promise().get_id();
    return coroutine;
  }
  std::uint_least32_t await_resume() const noexcept
  {
    return _M_id;
  }
private:
  mutable std::uint_least32_t _M_id = 0;
};
}
}
#endif



