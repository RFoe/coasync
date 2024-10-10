#ifndef COASYNC_ASYNC_FN_INCLUDED
#define COASYNC_ASYNC_FN_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

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

template<typename>
struct is_async_fn
  : std::false_type {};
template<typename R, typename... Args>
struct is_async_fn<awaitable<R>(Args ...)>
: std::true_type {};
template <typename T>
inline constexpr bool is_async_fn_v
  = is_async_fn<T>::value;

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
  COASYNC_ATTRIBUTE((noreturn, always_inline))
	static awaitable<R> _S_empty(COASYNC_ATTRIBUTE((maybe_unused)) std::any* __any, COASYNC_ATTRIBUTE((maybe_unused)) Args ...__args) noexcept
  {
    COASYNC_TERMINATE();
  }
public:
  inline constexpr static std::size_t arity = sizeof...(Args);
  typedef awaitable<R> result_type;
  template <std::size_t I>
  struct argument
  {
    static_assert(I < arity);
    typedef typename std::tuple_element<I, std::tuple<Args ...>>::type type;
  };
  template <std::size_t Index>
  using argument_t = typename argument<Index>::type;

  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit async_fn() noexcept: _M_fn(), _M_call(_S_empty) {}
  template <typename F> requires (!is_async_fn_v<std::remove_cvref_t<F>>)
  COASYNC_ATTRIBUTE((always_inline))
  constexpr async_fn(F&& __fn) noexcept(std::is_nothrow_constructible_v<std::decay_t<F>, F&&>)
    : _M_fn(std::forward<F>(__fn))
    , _M_call(&delegate<F>::_S_fp)
  {
    static_assert((bool) detail::function_traits<std::decay_t<F>>::value);
    typedef typename detail::function_traits<std::decay_t<F>>::result result_type;
    static_assert((bool) awaitable_traits<result_type>::value);
  }
  constexpr async_fn(async_fn const&) = delete;
  COASYNC_ATTRIBUTE((always_inline)) constexpr async_fn(async_fn&& other)
  noexcept(std::is_nothrow_move_constructible_v<result_type>)
    : _M_fn(std::move(other._M_fn)), _M_call(std::exchange(other._M_call, _S_empty))
  {}
  constexpr async_fn& operator=(async_fn const&) noexcept(false) = delete;
  COASYNC_ATTRIBUTE((always_inline)) async_fn& operator=(async_fn&& other) noexcept(std::is_nothrow_move_assignable_v<result_type>)
  {
    _M_fn = std::move(other._M_fn);
    _M_call = std::exchange(other._M_call, _S_empty);
    return (*this);
  }

  template <typename F> requires (!is_async_fn_v<std::remove_cvref_t<F>>)
  COASYNC_ATTRIBUTE((always_inline))
  constexpr async_fn& operator=(F&& __fn) noexcept(std::is_nothrow_constructible_v<std::decay_t<F>, F&&>)
  {
    static_assert((bool)detail::function_traits<std::decay_t<F>>::value);
    typedef typename detail::function_traits<std::decay_t<F>>::result result_type;
    static_assert((bool)awaitable_traits<result_type>::value);
    _M_fn = std::forward<F>(__fn);
    _M_call = &delegate<F>::_S_fp;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) explicit operator COASYNC_API bool() const noexcept
  {
    return _M_fn.has_value() and _M_call != _S_empty;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) const std::type_info& COASYNC_API target_type() const noexcept
  {
    return _M_fn.type();
  }
  template <typename T>
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) T* COASYNC_API target() noexcept
  {
    return std::any_cast<T>(std::addressof(_M_fn));
  }
  template <typename T>
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) const T* COASYNC_API target() const noexcept
  {
    return static_cast<T const*>(std::any_cast<T>(std::addressof(_M_fn)));
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  awaitable<R> COASYNC_API operator()(Args... args) noexcept
  {
    COASYNC_ASSERT(_M_fn.has_value());
    COASYNC_ATTRIBUTE((assume(_M_call != _S_empty)));
    return (* _M_call)(&_M_fn, std::forward<Args>(args)...);
  }
  COASYNC_ATTRIBUTE((always_inline)) void COASYNC_API swap(async_fn& other) noexcept
  {
    std::swap(_M_fn, other._M_fn);
    std::swap(_M_call, other._M_call);
  }
  COASYNC_ATTRIBUTE((always_inline)) friend void
  COASYNC_API swap(async_fn& __x, async_fn& __y) noexcept(noexcept(__x.swap(__y)))
  {
    __x.swap(__y);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) friend bool
  COASYNC_API operator==(const async_fn& __x, std::nullptr_t) noexcept
  {
    return __x._M_fn.has_value() and __x._M_call == _S_empty;
  }
  COASYNC_ATTRIBUTE((always_inline)) void COASYNC_API reset() noexcept(noexcept(this->_M_fn.reset())) {
		_M_fn.reset();
		_M_call = _S_empty;
	}
private:
  COASYNC_ATTRIBUTE((no_unique_address)) std::any _M_fn;
  awaitable<R>(* 	_M_call)(std::any*, Args ...);
};
#if defined(__cpp_deduction_guides)
template <typename F>
async_fn(F&&)
-> async_fn<typename detail::function_traits<F>::function>;
#endif
}

#endif
