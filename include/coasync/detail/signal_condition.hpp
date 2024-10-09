#ifndef COASYNC_SIGNAL_CONDITION_INCLUDED
#define COASYNC_SIGNAL_CONDITION_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "config.hpp"
#include <atomic>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{

template <std::size_t asynchronous_tag>
struct [[nodiscard]] signal_condition
{
  alignas(64) static inline /* volatile */ std::atomic_flag _S_flag {};
  static COASYNC_ATTRIBUTE((always_inline)) void _S_set([[maybe_unused]] int signum) noexcept
  {
    COASYNC_ASSERT(signum == asynchronous_tag);
    std::atomic_thread_fence(std::memory_order_relaxed);
    std::atomic_flag_test_and_set_explicit(&_S_flag, std::memory_order_release);
  }
};

}
}

#endif
