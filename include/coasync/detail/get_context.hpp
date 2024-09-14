#ifndef __COASYNC_GET_CONTEXT_INCLUDED
#define __COASYNC_GET_CONTEXT_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "awaitable_frame.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
/// Gets the scheduler reference associated with the current coroutine
struct get_context
{
  bool await_ready() const noexcept
  {
    return false;
  }
  template <typename Ref, typename Alloc>
  std::coroutine_handle<>
  await_suspend(std::coroutine_handle<awaitable_frame<Ref, Alloc>> coroutine) const noexcept
  {
    _M_context = coroutine.promise().get_context();
    return coroutine;
  }
  execution_context& await_resume() const noexcept
  {
    return * _M_context;
  }
private:
  mutable execution_context* _M_context = nullptr;
};
}
}
#endif
