#ifndef COASYNC_FRAME_DELEGATE_INCLUDED
#define COASYNC_FRAME_DELEGATE_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "awaitable_frame.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
/// The address of the coroutine_handle is strongly related to the alignment size of
/// the coroutine_traits<...>::promise_type, so even if there is an inheritance
/// relationship between promises, inconsistent alignment lengths can lead to incorrect
/// addresses after conversion. To put it simply, assuming struct PromiseA :
/// 	public PromiseB, and you have coroutine_handle<PromiseB> b;, it is not allowed
///		to use coroutine_handle<PromiseA>::from_address(b.address()) or coroutine_handle<PromiseA>::from_promise(b.promise()).
/// This means that due to potential differences in memory alignment between derived
/// and base classes, attempting to reinterpret the address or the promise object
/// from one class to another within the context of coroutines can result in undefined behavior or errors.
struct frame_delegate
{
/// notice that:
/// for the method [static constexpr coroutine_handle from_address( void *addr )]
/// the behavior is undefined if addr is neither a null pointer value
/// nor an underlying address of a coroutine_handle. The behavior is also undefined
/// if the addr is an underlying address of a std::coroutine_handle<P1>, where both
/// Promise and P1 are not void, and P1 is different from Promise.
  enum Op
  {
    Op_destroy, Op_done
  };
  template <typename Ref, typename Alloc>
  struct delegator
  {
    COASYNC_ATTRIBUTE((always_inline))
    static void S_delegate(
      Op 											opcode,
      std::coroutine_handle<awaitable_frame_base> frame_base,
      /// awaitable_frame_base: type-erasing not supported for std::coroutine_handle<P>::done/destroy
      COASYNC_ATTRIBUTE((maybe_unused)) void* 	payload)
    /// use void* for type-erasing
    {
      /// The destory and done methods of the coroutine have strong requirements
      /// for type alignment in specific implementations;
      auto frame = std::coroutine_handle<awaitable_frame<Ref, Alloc>>::from_address(frame_base.address());
      switch(opcode)
        {
        case Op_done:
          COASYNC_ATTRIBUTE((likely))
          * static_cast<bool*>(payload) = frame.done();
          break;
        case Op_destroy:
          COASYNC_ATTRIBUTE((unlikely))
          frame.destroy();
          break;
        }
      /// notice that coroutine resume is OK with uncompatible promise_type
    }
  };
  typedef void(* delegate_type)(Op, std::coroutine_handle<awaitable_frame_base>, void*);
};
}
}
#endif
