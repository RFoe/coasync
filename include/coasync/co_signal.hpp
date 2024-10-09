#ifndef COASYNC_CO_SIGNAL_INCLUDED
#define COASYNC_CO_SIGNAL_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "co_spawn.hpp"
#include "detail/signal_condition.hpp"
#include "detail/service/flag_service.hpp"
#include "detail/suspendible.hpp"
#include <csignal>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
template <std::size_t asynchronous_tag, typename execution_context, typename Fn>
requires std::is_invocable_r_v<void, std::decay_t<Fn>, int>
void COASYNC_API co_signal(execution_context& context, Fn&& fn)
{
  return (void) co_spawn(context, []
#if __cplusplus >= 202207L
                         COASYNC_ATTRIBUTE((nodiscard))
#endif
                         (std::function<void(int)> __fn)
                         mutable noexcept(std::is_nothrow_invocable_r_v<void, std::decay_t<Fn>, int>) -> awaitable<void>
  {
    void(*__fp)(int) = std::signal(static_cast<int>(asynchronous_tag), &detail::signal_condition<asynchronous_tag>::_S_set);
    std::atomic_flag /* volatile */& __flag = detail::signal_condition<asynchronous_tag>::_S_flag;
    co_await detail::suspendible<detail::flag_service>()(__flag);
    (void) (__fn)(static_cast<int>(asynchronous_tag));
    (void) std::signal(static_cast<int>(asynchronous_tag), __fp);
		std::atomic_flag_clear_explicit(&detail::signal_condition<asynchronous_tag>::_S_flag, std::memory_order_release);
  }(std::forward<Fn>(fn)), detail::use_detach_t{});
}

}

#endif
