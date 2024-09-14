#ifndef COASYNC_CO_BIND_INCLUDED
#define COASYNC_CO_BIND_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "co_spawn.hpp"
#include <functional>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
/// Encapsulates the synchronous function as asynchronous awaitable type
template <typename execution_context, typename Fn, typename... Args>
requires std::is_invocable_v<Fn, Args ...>
COASYNC_ATTRIBUTE((nodiscard))
awaitable<std::invoke_result_t<Fn, Args ...>>
    COASYNC_API co_bind(execution_context& context, Fn&& fn, Args&& ... args)
{
  using invoke_result_t = std::invoke_result_t<Fn, Args ...>;
  std::packaged_task<invoke_result_t()> 	packaged_task {std::bind(std::forward<Fn>(fn), std::forward<Args>(args) ...)};
  std::future<invoke_result_t> 						future = packaged_task.get_future();
  co_spawn(context, [](std::packaged_task<invoke_result_t()> packaged_task) -> awaitable<void>
  {
    co_return (void)packaged_task();
  }(std::move(packaged_task)), use_detach);
  return [](std::future<invoke_result_t> future) -> awaitable<invoke_result_t>
  {
    co_await detail::suspendible<detail::future_service>()(future);
    if constexpr(std::is_void_v<invoke_result_t>) co_return;
    else co_return future.get();
  }(std::move(future));
}
}
#endif
