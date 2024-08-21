#ifndef __COASYNC_GET_STOKEN_INCLUDED
#define __COASYNC_GET_STOKEN_INCLUDED
#include "awaitable_frame.hpp"
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
/// Gets the cancel token associated with the current coroutine
struct get_stop_token
{
  bool await_ready() const noexcept
  {
    return false;
  }
  template <typename Ref, typename Alloc>
  std::coroutine_handle<>
  await_suspend(std::coroutine_handle<awaitable_frame<Ref, Alloc>> coroutine) const noexcept
  {
    _M_stoken = coroutine.promise().get_stop_token();
    return coroutine;
  }
  std::stop_token await_resume() const noexcept
  {
    return std::move(_M_stoken);
  }
private:
  mutable std::stop_token _M_stoken {};
};
}
}
#endif
