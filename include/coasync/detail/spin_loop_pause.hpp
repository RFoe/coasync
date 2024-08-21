#ifndef COASYNC_SPIN_LOOP_PAUSE_INCLUDED
#define COASYNC_SPIN_LOOP_PAUSE_INCLUDED
/// Used to pause the current thread in a spin lock or other spin waiting mechanism
/// to reduce CPU utilization. The function implements cross-platform spin wait
/// pause instruction to improve performance on multi-core processors.
#include "config.hpp"
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#  if defined(MSVC)
#    include <intrin.h>
#  endif
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
COASYNC_ATTRIBUTE((always_inline)) static void __spin_loop_pause() noexcept
{
#  if defined(MSVC)
/// If it is a hyperthreaded CPU machine, the CPU can be transferred to other threads;
/// Because current CPU-core has nothing serious to do at present, it is better to
/// slow down the pace of time and let others do more; It is a compilation barrier,
/// making volatile changes in the kernel largely unnecessary.
  _mm_pause();
#  else
/// These built-in functions are available for the x86-32 and x86-64 family of computers,
/// depending on the command-line switches used.
  __builtin_ia32_pause();
/// Generates the pause machine instruction with a compiler memory barrier.
#  endif
}
}
}
#elif defined(__arm__) || defined(__aarch64__) || defined(_M_ARM64)
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
COASYNC_ATTRIBUTE((always_inline)) static void __spin_loop_pause() noexcept
{
#  if (                                                                                            \
    defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__)              \
    || defined(__ARM_ARCH_6T2__) || defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__)            \
    || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)            \
    || defined(__ARM_ARCH_8A__) || defined(__aarch64__))
  asm volatile("yield" ::: "memory");
#  elif defined(_M_ARM64)
  __yield();
#  else
  asm volatile("nop" ::: "memory");
#  endif
}
}
}
#else
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
COASYNC_ATTRIBUTE((always_inline)) static void __spin_loop_pause() noexcept
{
}
}
}
#endif
#endif
