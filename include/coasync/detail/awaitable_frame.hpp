#ifndef COASYNC_AWAITABLE_FRAME_INCLUDED
#define COASYNC_AWAITABLE_FRAME_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "awaitable_frame_base.hpp"
#include "awaitable_frame_alloc.hpp"
#include "manual_lifetime.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
template <typename Ref, typename Alloc>
struct awaitable_frame final
  : public awaitable_frame_base
  , public awaitable_frame_alloc<Alloc>
{
  COASYNC_ATTRIBUTE((noreturn))
	static std::coroutine_handle<awaitable_frame> get_return_object_on_allocation_failure()
  {
    throw std::bad_alloc(); // or, return Coroutine(nullptr);
  }
  std::coroutine_handle<awaitable_frame> get_return_object() noexcept
  {
    return std::coroutine_handle<awaitable_frame>::from_promise(*this);
  }
  template <typename U>
  void return_value(U&& value)
  noexcept(std::is_nothrow_constructible_v<Ref, U>)
  requires (not std::is_reference_v<Ref>)
  {
    static_assert(std::is_constructible_v<Ref, U>);
    _M_value.construct(std::forward<U&&>(value));
  }
  void return_value(Ref&& value)
  noexcept(std::is_nothrow_move_constructible_v<Ref>)
  {
    static_assert(std::is_move_constructible_v<Ref>);
    _M_value.construct(static_cast<Ref &&>(value));
  }
  /// An interface that can only be called once to get the return value of the coroutine
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  Ref COASYNC_API get_value() noexcept
  {
    Ref value = static_cast<Ref &&>(_M_value.get());
    _M_value.destruct();
    return value;
  }
  using awaitable_frame_alloc<Alloc>::operator new;
  using awaitable_frame_alloc<Alloc>::operator delete;
private:
  COASYNC_ATTRIBUTE((no_unique_address)) manual_lifetime<Ref> _M_value;
};
template <typename Alloc>
struct awaitable_frame<void, Alloc> final
  : public awaitable_frame_base
  , public awaitable_frame_alloc<Alloc>
{
  std::coroutine_handle<awaitable_frame> get_return_object() noexcept
  {
    return std::coroutine_handle<awaitable_frame>::from_promise(*this);
  }
  void return_void() const noexcept {}
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API get_value() const noexcept {}
  using awaitable_frame_alloc<Alloc>::operator new;
  using awaitable_frame_alloc<Alloc>::operator delete;
};

template <typename> struct awaitable_frame_traits: std::false_type {};
template <typename Ref, typename Alloc>
struct awaitable_frame_traits<awaitable_frame<Ref, Alloc>>: std::true_type
{
  typedef Ref 	value_type;
  typedef Alloc allocator_type;
};

}
}
#endif
