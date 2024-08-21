#ifndef COASYNC_CO_SPAWN_INCLUDED
#define COASYNC_CO_SPAWN_INCLUDED
#include "detail/suspendible.hpp"
#include "detail/service/future_service.hpp"
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
/// Spawn a new coroutined-based thread of execution.
/// a high-level wrapper over the Boost.Coroutine library. This function enables programs
/// to implement asynchronous logic in a synchronous manner
struct use_detach_t {};
struct use_future_t {};
struct use_awaitable_t {};
/// The completion token will tag the notification how the thread of execution would
/// be completed.
}
inline constexpr detail::use_detach_t use_detach;
inline constexpr detail::use_future_t use_future;
inline constexpr detail::use_awaitable_t use_awaitable;
template <typename execution_context, typename Ref, typename Alloc>
void COASYNC_API co_spawn(
  execution_context& 				context,
  awaitable<Ref, Alloc> 		a,
  COASYNC_ATTRIBUTE((maybe_unused)) detail::use_detach_t)
{
  std::coroutine_handle<detail::awaitable_frame<Ref, Alloc>>
      frame = a.release_coroutine();
  context.push_frame_to_lifetime(frame);
  context.push_frame_to_executor(std::coroutine_handle<detail::awaitable_frame_base>::from_address(frame.address()));
}
template <typename execution_context, typename Ref, typename Alloc>
COASYNC_ATTRIBUTE((nodiscard))
std::future<Ref> COASYNC_API co_spawn(
  execution_context& 			context,
  awaitable<Ref, Alloc> 	a,
  COASYNC_ATTRIBUTE((maybe_unused)) detail::use_future_t)
{
  std::promise<Ref> promise;
  std::future<Ref>  future = promise.get_future();
  co_spawn(context, [](awaitable<Ref, Alloc> a, std::promise<Ref> promise) -> awaitable<void>
  {
    if constexpr(not std::is_void_v<Ref>)
      promise.set_value(static_cast<Ref&&>(co_await a));
    else
      {
        co_await a;
        promise.set_value();
      }
  }(std::move(a), std::move(promise)), use_detach);
  return future;
}
template <typename execution_context, typename Ref, typename Alloc>
COASYNC_ATTRIBUTE((nodiscard))
awaitable<Ref> COASYNC_API co_spawn(
  execution_context& 			context,
  awaitable<Ref, Alloc> 	a,
  COASYNC_ATTRIBUTE((maybe_unused)) detail::use_awaitable_t)
{
  return [](std::future<Ref> future) -> awaitable<Ref>
  {
    co_await detail::suspendible<detail::future_service>()(future);
    if constexpr(not std::is_void_v<Ref>) co_return future.get();
    else
      {
        future.wait();
        co_return;
      }
  }(co_spawn(context, std::move(a), use_future));
}
}
#endif
