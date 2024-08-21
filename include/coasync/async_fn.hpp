#ifndef COASYNC_ASYNC_FN_INCLUDED
#define COASYNC_ASYNC_FN_INCLUDED

#include "awaitable.hpp"
#include "detail/meta/function_traits.hpp"
#include <any>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
template <typename> struct async_fn;
/// intended to wrap a callable object (such as a function, lambda, or functor) and
/// provide an awaitable interface for asynchronous execution. It allows the wrapped
/// callable to be invoked asynchronously using the operator() and supports various
/// types of callables.
template <typename R, typename... Args>
struct async_fn<awaitable<R>(Args ...)>
{
/// a general-purpose polymorphic function wrapper that generates awaitable. Instances
/// of async_fn can store, copy, and invoke any CopyConstructible asynchronous Callable
/// target -- functions (via pointers thereto), lambda expressions, bind expressions,
/// or other function objects, as well as pointers to member functions and pointers
/// to data members.
/// The stored callable object is called the target of async_fn. If a async_fn
/// contains no target, it is called empty. Invoking the target of an empty async_fn
/// results that std::terminate ocurr. async_fn satisfies the requirements of
/// CopyConstructible and CopyAssignable.
private:
  template <typename F>
  struct delegate
  {
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    static awaitable<R> _S_fp(std::any* __any, Args ...__args)
    noexcept(std::is_nothrow_invocable_v<F, Args ...>)
    COASYNC_ATTRIBUTE((gnu::nonnull))
    {
      return (* std::any_cast<F>(__any))(std::forward<Args>(__args)...);
    }
  };
public:
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit async_fn() noexcept: _M_fn(), _M_call(nullptr) {}
  template <typename F>
  COASYNC_ATTRIBUTE((always_inline))
  constexpr async_fn(F&& __fn) noexcept(std::is_nothrow_constructible_v<std::decay_t<F>, F&&>)
    : _M_fn(std::forward<F>(__fn))
    , _M_call(&delegate<F>::_S_fp)
  {
    static_assert(detail::function_traits<F>::value);
    typedef typename detail::function_traits<F>::result result_type;
    static_assert(awaitable_traits<result_type>::value);
  }
  template <typename F>
  COASYNC_ATTRIBUTE((always_inline))
  constexpr async_fn& operator=(F&& __fn) noexcept(std::is_nothrow_constructible_v<std::decay_t<F>, F&&>)
  {
    static_assert(detail::function_traits<F>::value);
    typedef typename detail::function_traits<F>::result result_type;
    static_assert(awaitable_traits<result_type>::value);
    _M_fn = std::forward<F>(__fn);
    _M_call = &delegate<F>::_S_fp;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  awaitable<R> operator()(Args... args) noexcept
  {
    COASYNC_ASSERT(_M_fn.has_value());
    COASYNC_ATTRIBUTE((assume(_M_call != nullptr)));
    return (* _M_call)(&_M_fn, std::forward<Args>(args)...);
  }
  COASYNC_ATTRIBUTE((always_inline)) void swap(async_fn& other) noexcept {
		std::swap(_M_fn, other._M_fn);
		std::swap(_M_call, other._M_call);
	}
private:
  COASYNC_ATTRIBUTE((no_unique_address)) std::any _M_fn;
  awaitable<R>(* 	_M_call)(std::any*, Args ...);
};
template <typename F>
async_fn(F&&)
-> async_fn<typename detail::function_traits<F>::function>;
}

#endif
