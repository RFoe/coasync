#ifndef COASYNC_AWAITABLE_INCLUDED
#define COASYNC_AWAITABLE_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "detail/awaitable_frame.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
/// The return type of a coroutine or asynchronous operation.
/// Support for C++20 Coroutines is provided via the awaitable class template,
/// the use_awaitable completion token, and the co_spawn() function. These facilities
/// allow programs to implement asynchronous logic in a synchronous manner,
/// in conjunction with the co_await keyword
template <typename Ref, typename Alloc = std::allocator<std::byte>>
struct awaitable final
{
  template <typename OtherAlloc>
  struct rebind_allocator
  {
    typedef awaitable<Ref, OtherAlloc> type;
  };
  template <typename OtherRef>
  struct rebind_value
  {
    typedef awaitable<OtherRef, Alloc> type;
  };
  typedef Ref 		value_type;
  typedef Alloc 	allocator_type;
/// awaitable type is used to manage the lifetime of coroutine:
///    providing exception safety with dynamic lifetime, by guaranteeing destroy
/// 			coroutine-handle on both normal exit and exit through exception.
///    passing ownership of uniquely-owned objects with dynamic lifetime into
/// 			functions using moving assignment/construction.
///    acquiring ownership of uniquely-owned objects with dynamic lifetime from
/// 			functions using moving assignment/construction.
///    as the element type in move-aware containers, such as std::vector, which
/// 			hold coroutine_handle to dynamically/statically-allocated coroutine promise
  awaitable(awaitable&& other)
  noexcept: _M_coroutine(std::exchange(other._M_coroutine, nullptr))
  {
  }
  /// Constructs a awaitable by transferring ownership from other to *this and
  /// stores the null coroutine in other.
  awaitable& operator=(awaitable&& other) noexcept
  {
    if (&other != this)
      _M_coroutine = std::exchange(other._M_coroutine, nullptr);
    return (*this);
  }
  ~ awaitable()
  {
    if (not _M_coroutine)
      return;
    _M_coroutine.destroy();
  }
  constexpr awaitable& operator=(awaitable const&) = delete;
  void swap(awaitable& other) noexcept
  {
    std::swap(_M_coroutine, other._M_coroutine);
  }
  struct await_result
  {
    bool await_ready() const noexcept
    {
      return _M_coroutine.done();
    }
    template <typename OtherRef, typename OtherAlloc>
    std::coroutine_handle<> await_suspend(
      std::coroutine_handle<detail::awaitable_frame<OtherRef, OtherAlloc>> coroutine) noexcept
    {
      /// There is no way to preserve the awaitable_frame template type itself,
      /// and we need to use a generic type to use them, in this case, need to
      /// erase the original type of the object
      auto previous_frame = std::coroutine_handle<detail::awaitable_frame_base>::from_address(coroutine.address());
      _M_coroutine.promise().push_frame(previous_frame);
      return _M_coroutine;
    }
    Ref await_resume() const
    {
      _M_coroutine.promise().rethrow_exception();
      return _M_coroutine.promise().get_value();
    }
    std::coroutine_handle<detail::awaitable_frame<Ref, Alloc>> _M_coroutine;
  };
  await_result operator co_await() const noexcept
  {
    return await_result { _M_coroutine };
  }
  awaitable(std::coroutine_handle<detail::awaitable_frame<Ref, Alloc>> coroutine) noexcept
    : _M_coroutine(coroutine)
  {
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  std::coroutine_handle<detail::awaitable_frame<Ref, Alloc>>
      COASYNC_API get_coroutine() const noexcept
  {
    return _M_coroutine;
  }
  /// Releases the ownership of the managed coroutine promise, if any.
	/// get_coroutine() returns nullptr after the call.
	/// The caller is responsible for destroy the coroutine
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  std::coroutine_handle<detail::awaitable_frame<Ref, Alloc>>
      COASYNC_API release_coroutine() noexcept
  {
    return std::exchange(_M_coroutine, nullptr);
  }
private:
  std::coroutine_handle<detail::awaitable_frame<Ref, Alloc>> _M_coroutine;
};
template <typename> struct awaitable_traits: std::false_type {};
template <typename Ref, typename Alloc>
struct awaitable_traits<awaitable<Ref, Alloc>>: std::true_type {
	typedef Ref 	value_type;
	typedef Alloc allocator_type;
};
}
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) std
{
template<typename Ref, typename Alloc, typename... Args>
struct coroutine_traits<coasync::awaitable<Ref, Alloc>, Args...>
{
  using promise_type = coasync::detail::awaitable_frame<Ref, Alloc>;
};
}
#endif
