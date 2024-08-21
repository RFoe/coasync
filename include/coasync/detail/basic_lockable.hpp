#ifndef __COASYNC_BASIC_LOCKABLE_INCLUDED
#define __COASYNC_BASIC_LOCKABLE_INCLUDED
#include "config.hpp"
#include "spin_loop_pause.hpp"
#include <atomic>
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
/// Simple spin lock with atomic variables
struct basic_lockable
{
  COASYNC_ATTRIBUTE((always_inline)) void lock() noexcept
  {
    bool desired = false;
    do
      {
#if defined(__cpp_lib_atomic_wait)
        _M_locked.wait(true, std::memory_order_acquire);
/// Performs atomic waiting operations. Behaves as if it repeatedly performs the following steps:
///   Compare the value representation of this->load(order) with that of old.
///   If those are equal, then blocks until *this is notified by notify_one() or notify_all(),
///		or the thread is unblocked spuriously. Otherwise, returns.
/// These functions are guaranteed to return only if value has changed, even if underlying
/// implementation unblocks spuriously.
#else
				detail::__spin_loop_pause();
#endif
        desired = false;
      }
    while (not _M_locked.compare_exchange_weak(desired, true,
           std::memory_order_release, std::memory_order_relaxed));
  }
  COASYNC_ATTRIBUTE((always_inline)) void unlock() noexcept
  {
    if (not _M_locked.load(std::memory_order_acquire)) [[unlikely]]
      return;
    _M_locked.store(false, std::memory_order_release);
#if defined(__cpp_lib_atomic_wait)
/// Performs atomic notifying operations.
/// If there is a thread blocked in atomic waiting operation (i.e. wait()) on *this,
/// then unblocks at least one such thread; otherwise does nothing.
		_M_locked.notify_one();
#endif
  }
private:
  std::atomic_bool _M_locked { false };
};
}
}
#endif
