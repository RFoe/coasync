#ifndef COASYNC_GENERATOR_INCLUDED
#define COASYNC_GENERATOR_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "detail/config.hpp"
#if __has_include(<generator>) && defined(__cpp_lib_generator)
#include <generator>
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
template <typename _Ref, typename _Value, typename _Alloc>
using generator = std::generator;
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) ranges
{
template <typename _Rng, typename _Allocator>
using elements_of = std::ranges::elements_of;
}
}
#else

#include "detail/manual_lifetime.hpp"
#include "detail/awaitable_frame_alloc.hpp"
#include <utility> /// for std::exchange

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
struct use_allocator_arg {};
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) ranges
{

template <typename _Rng, typename _Allocator = use_allocator_arg>
struct elements_of
{
  COASYNC_ATTRIBUTE((always_inline)) explicit constexpr elements_of(_Rng&& __rng) noexcept
  requires std::is_default_constructible_v<_Allocator>
    :
    __range(static_cast<_Rng&&>(__rng))
  {}

  COASYNC_ATTRIBUTE((always_inline)) constexpr elements_of(_Rng&& __rng, _Allocator&& __alloc) noexcept
    : __range((_Rng&&)__rng), __alloc((_Allocator&&)__alloc) {}

  COASYNC_ATTRIBUTE((always_inline)) constexpr elements_of(elements_of&&) noexcept = default;

  constexpr elements_of(const elements_of&) = delete;
  constexpr elements_of& operator=(const elements_of&) = delete;
  constexpr elements_of& operator=(elements_of&&) = delete;

  COASYNC_ATTRIBUTE((nodiscard, always_inline)) constexpr _Rng&& get() noexcept
  {
    return static_cast<_Rng&&>(__range);
  }

  COASYNC_ATTRIBUTE((nodiscard, always_inline)) constexpr _Allocator get_allocator() const noexcept
  {
    return __alloc;
  }

private:
  COASYNC_ATTRIBUTE((no_unique_address)) _Allocator __alloc; // \expos
  _Rng&& __range;  // \expos
};

template <typename _Rng>
elements_of(_Rng&&) -> elements_of<_Rng>;

template <typename _Rng, typename Allocator>
elements_of(_Rng&&, Allocator&&) -> elements_of<_Rng, Allocator>;
} // namespace ranges

template <typename _Ref,
          typename _Value = std::remove_cvref_t<_Ref>,
          typename _Allocator = use_allocator_arg>
class generator;

template<typename _Ref>
struct generator_promise_base
{
  template <typename _Ref2, typename _Value, typename _Alloc>
  friend class generator;

  generator_promise_base* _M_root;
  std::coroutine_handle<> _M_parent_or_leaf;
  // Note: Using manual_lifetime here to avoid extra calls to exception_ptr
  // constructor/destructor in cases where it is not needed (i.e. where this
  // generator coroutine is not used as a nested coroutine).
  // This member is lazily constructed by the __yield_sequence_awaiter::await_suspend()
  // method if this generator is used as a nested generator.
  detail::manual_lifetime<std::exception_ptr> _M_expection;
  detail::manual_lifetime<_Ref> _M_value;

  COASYNC_ATTRIBUTE((always_inline)) explicit generator_promise_base(std::coroutine_handle<> thisCoro) noexcept
    : _M_root(this)
    , _M_parent_or_leaf(thisCoro)
  {}

  COASYNC_ATTRIBUTE((always_inline)) ~generator_promise_base() noexcept
  {
    if (_M_root != this) COASYNC_ATTRIBUTE((likely))
      // This coroutine was used as a nested generator and so will
      // have constructed its _M_expection member which needs to be
      // destroyed here.
      _M_expection.destruct();
  }

  std::suspend_always initial_suspend() noexcept
  {
    return {};
  }

  void return_void() noexcept {}

  void unhandled_exception()
  {
    if (_M_root != this) COASYNC_ATTRIBUTE((likely))
      _M_expection.get() = std::current_exception();
    else COASYNC_ATTRIBUTE((unlikely))
      std::rethrow_exception(std::current_exception());
  }

  // Transfers control back to the parent of a nested coroutine
  struct __final_awaiter
  {
    bool await_ready() noexcept
    {
      return false;
    }

    template <typename _Promise>
    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<_Promise> __h) noexcept
    {
      _Promise& __promise = __h.promise();
      generator_promise_base& __root = *__promise._M_root;
      if (&__root != &__promise) COASYNC_ATTRIBUTE((unlikely))
        {
          auto __parent = __promise._M_parent_or_leaf;
          __root._M_parent_or_leaf = __parent;
          return __parent;
        }
      return std::noop_coroutine();
    }

    void await_resume() noexcept {}
  };

  __final_awaiter final_suspend() noexcept
  {
    return {};
  }

  std::suspend_always yield_value(_Ref&& __x)
  noexcept(std::is_nothrow_move_constructible_v<_Ref>)
  {
    _M_root->_M_value.construct((_Ref&&)__x);
    return {};
  }

  template <typename _T>
  requires
  (!std::is_reference_v<_Ref>)&&
  std::is_convertible_v<_T, _Ref>
  std::suspend_always yield_value(_T&& __x)
  noexcept(std::is_nothrow_constructible_v<_Ref, _T>)
  {
    _M_root->_M_value.construct((_T&&)__x);
    return {};
  }

  template <typename _Gen>
  struct __yield_sequence_awaiter
  {
    _Gen __gen_;

    COASYNC_ATTRIBUTE((always_inline)) __yield_sequence_awaiter(_Gen&& __g) noexcept
    // Taking ownership of the generator ensures frame are destroyed
    // in the reverse order of their execution.
      : __gen_(static_cast<_Gen&&>(__g))
    {
    }

    bool await_ready() noexcept
    {
      return false;
    }

    // set the parent, root and exceptions pointer and
    // resume the nested
    template<typename _Promise>
    std::coroutine_handle<>
    await_suspend(std::coroutine_handle<_Promise> __h) noexcept
    {
      generator_promise_base& __current = __h.promise();
      generator_promise_base& __nested = *__gen_.__get_promise();
      generator_promise_base& __root = *__current._M_root;

      __nested._M_root = __current._M_root;
      __nested._M_parent_or_leaf = __h;

      // Lazily construct the _M_expection member here now that we
      // know it will be used as a nested generator. This will be
      // destroyed by the promise destructor.
      __nested._M_expection.construct();
      __root._M_parent_or_leaf = __gen_.__get_coro();

      // Immediately resume the nested coroutine (nested generator)
      return __gen_.__get_coro();
    }

    void await_resume()
    {
      generator_promise_base& __nestedPromise = *__gen_.__get_promise();
      if (__nestedPromise._M_expection.get())
        {
          std::rethrow_exception(std::move(__nestedPromise._M_expection.get()));
        }
    }
  };

  template <typename _OValue, typename _OAlloc>
  __yield_sequence_awaiter<generator<_Ref, _OValue, _OAlloc>>
      yield_value(ranges::elements_of<generator<_Ref, _OValue, _OAlloc>> __g) noexcept
  {
    return std::move(__g).get();
  }

  template <std::ranges::range _Rng, typename _Allocator>
  __yield_sequence_awaiter<generator<_Ref, std::remove_cvref_t<_Ref>, _Allocator>>
      yield_value(ranges::elements_of<_Rng, _Allocator>&& __x)
  {
    return [](std::allocator_arg_t, _Allocator alloc, auto&& __rng) -> generator<_Ref, std::remove_cvref_t<_Ref>, _Allocator>
    {
      for(auto&& e: __rng)
        co_yield static_cast<decltype(e)>(e);
    }(std::allocator_arg, __x.get_allocator(), std::forward<_Rng>(__x.get()));
  }

  COASYNC_ATTRIBUTE((always_inline)) void resume()
  {
    _M_parent_or_leaf.resume();
  }

  // Disable use of co_await within this coroutine.
  void await_transform() = delete;
};

template<typename _Generator, typename _ByteAllocator, bool _ExplicitAllocator = false>
struct __generator_promise;

template<typename _Ref, typename _Value, typename _Alloc, typename _ByteAllocator, bool _ExplicitAllocator>
struct __generator_promise<generator<_Ref, _Value, _Alloc>, _ByteAllocator, _ExplicitAllocator> final
  : public generator_promise_base<_Ref>
  , public detail::awaitable_frame_alloc<_ByteAllocator>
{
  __generator_promise() noexcept
    : generator_promise_base<_Ref>(std::coroutine_handle<__generator_promise>::from_promise(*this))
  {}

  generator<_Ref, _Value, _Alloc> get_return_object() noexcept
  {
    return generator<_Ref, _Value, _Alloc>
    {
      std::coroutine_handle<__generator_promise>::from_promise(*this)
    };
  }

  using generator_promise_base<_Ref>::yield_value;

  template <std::ranges::range _Rng>
  typename generator_promise_base<_Ref>::template __yield_sequence_awaiter<generator<_Ref, _Value, _Alloc>>
  yield_value(ranges::elements_of<_Rng>&& __x)
  {
    static_assert (!_ExplicitAllocator,
                   "This coroutine has an explicit allocator specified with std::allocator_arg so an allocator needs to be passed "
                   "explicitely to std::elements_of");
    return [](auto&& __rng) -> generator<_Ref, _Value, _Alloc>
    {
      for(auto&& e: __rng)
        co_yield static_cast<decltype(e)>(e);
    }(std::forward<_Rng>(__x.get()));
  }
};

template<typename _Alloc>
using __byte_allocator_t = typename std::allocator_traits<std::remove_cvref_t<_Alloc>>::template rebind_alloc<std::byte>;

// TODO :  make layout compatible promise casts possible
template <typename _Ref, typename _Value, typename _Alloc>
class generator
{
  using __byte_allocator = __byte_allocator_t<_Alloc>;
public:
  using promise_type = __generator_promise<generator<_Ref, _Value, _Alloc>, __byte_allocator>;
  friend promise_type;
private:
  using __coroutine_handle = std::coroutine_handle<promise_type>;
public:

  COASYNC_ATTRIBUTE((always_inline)) generator() noexcept = default;

  COASYNC_ATTRIBUTE((always_inline)) generator(generator&& __other) noexcept
    : __coro_(std::exchange(__other.__coro_, {}))
  , __started_(std::exchange(__other.__started_, false))
  {
  }

  COASYNC_ATTRIBUTE((always_inline)) ~generator() noexcept
  {
    if (__coro_) COASYNC_ATTRIBUTE((likely))
      {
        if (__started_ && !__coro_.done()) COASYNC_ATTRIBUTE((unlikely))
          __coro_.promise()._M_value.destruct();
        __coro_.destroy();
      }
  }

  COASYNC_ATTRIBUTE((always_inline)) generator& operator=(generator&& g) noexcept
  {
    std::swap(g);
    return *this;
  }

  COASYNC_ATTRIBUTE((always_inline)) void swap(generator& __other) noexcept
  {
    std::swap(__coro_, __other.__coro_);
    std::swap(__started_, __other.__started_);
  }

  class iterator
  {
  public:
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = _Value;
    using reference = _Ref;
    using pointer = std::add_pointer_t<_Ref>;

    COASYNC_ATTRIBUTE((always_inline)) iterator() noexcept = default;
    iterator(const iterator&) = delete;

    COASYNC_ATTRIBUTE((always_inline)) iterator(iterator&& __other) noexcept
      : __coro_(std::exchange(__other.__coro_, {}))
    {
    }

    COASYNC_ATTRIBUTE((always_inline)) iterator& operator=(iterator&& __other)
    {
      std::swap(__coro_, __other.__coro_);
      return *this;
    }

    COASYNC_ATTRIBUTE((always_inline)) ~iterator()
    {
    }

    COASYNC_ATTRIBUTE((nodiscard, always_inline)) friend bool operator==(const iterator& it, std::default_sentinel_t) noexcept
    {
      return it.__coro_.done();
    }

    COASYNC_ATTRIBUTE((always_inline)) iterator& operator++()
    {
      __coro_.promise()._M_value.destruct();
      __coro_.promise().resume();
      return *this;
    }
    COASYNC_ATTRIBUTE((always_inline)) void operator++(int)
    {
      (void)operator++();
    }

    COASYNC_ATTRIBUTE((nodiscard, always_inline)) reference operator*() const noexcept
    {
      return static_cast<reference>(__coro_.promise()._M_value.get());
    }

  private:
    friend generator;

    COASYNC_ATTRIBUTE((always_inline)) explicit iterator(__coroutine_handle __coro) noexcept
      : __coro_(__coro) {}

    __coroutine_handle __coro_;
  };

  COASYNC_ATTRIBUTE((nodiscard, always_inline)) iterator begin()
  {
    assert(__coro_);
    assert(!__started_);
    __started_ = true;
    __coro_.resume();
    return iterator{__coro_};
  }

  COASYNC_ATTRIBUTE((maybe_unused, always_inline)) std::default_sentinel_t end() noexcept
  {
    return {};
  }

private:
  COASYNC_ATTRIBUTE((always_inline)) explicit generator(__coroutine_handle __coro) noexcept
    : __coro_(__coro)
  {
  }

public: // to get around access restrictions for __yield_sequence_awaitable
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) std::coroutine_handle<> __get_coro() noexcept
  {
    return __coro_;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) promise_type* __get_promise() noexcept
  {
    return std::addressof(__coro_.promise());
  }

private:
  __coroutine_handle __coro_;
  bool __started_ = false;
};

// Specialisation for type-erased allocator implementation.
template <typename _Ref, typename _Value>
class generator<_Ref, _Value, use_allocator_arg>
{
  using __promise_base = generator_promise_base<_Ref>;
public:

  COASYNC_ATTRIBUTE((always_inline)) generator() noexcept
    : __promise_(nullptr)
    , __coro_()
    , __started_(false)
  {}

  COASYNC_ATTRIBUTE((always_inline)) generator(generator&& __other) noexcept
    : __promise_(std::exchange(__other.__promise_, nullptr))
    , __coro_(std::exchange(__other.__coro_, {}))
  , __started_(std::exchange(__other.__started_, false))
  {
  }

 COASYNC_ATTRIBUTE((always_inline))  ~generator() noexcept
  {
    if (__coro_) COASYNC_ATTRIBUTE((likely))
      {
        if (__started_ && !__coro_.done())  COASYNC_ATTRIBUTE((unlikely))
            __promise_->_M_value.destruct();
        __coro_.destroy();
      }
  }

  COASYNC_ATTRIBUTE((always_inline)) generator& operator=(generator g) noexcept
  {
    std::swap(g);
    return *this;
  }

  COASYNC_ATTRIBUTE((always_inline)) void swap(generator& __other) noexcept
  {
    std::swap(__promise_, __other.__promise_);
    std::swap(__coro_, __other.__coro_);
    std::swap(__started_, __other.__started_);
  }

  class iterator
  {
  public:
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = _Value;
    using reference = _Ref;
    using pointer = std::add_pointer_t<_Ref>;

    COASYNC_ATTRIBUTE((always_inline)) iterator() noexcept = default;
    iterator(const iterator&) = delete;

    COASYNC_ATTRIBUTE((always_inline)) iterator(iterator&& __other) noexcept
      : __promise_(std::exchange(__other.__promise_, nullptr))
      , __coro_(std::exchange(__other.__coro_, {}))
    {}

    COASYNC_ATTRIBUTE((always_inline)) iterator& operator=(iterator&& __other)
    {
      __promise_ = std::exchange(__other.__promise_, nullptr);
      __coro_ = std::exchange(__other.__coro_, {});
      return *this;
    }

    COASYNC_ATTRIBUTE((always_inline)) ~iterator() = default;

    COASYNC_ATTRIBUTE((nodiscard, always_inline)) friend bool operator==(const iterator& it, std::default_sentinel_t) noexcept
    {
      return it.__coro_.done();
    }

    COASYNC_ATTRIBUTE((always_inline)) iterator& operator++()
    {
      __promise_->_M_value.destruct();
      __promise_->resume();
      return *this;
    }

    COASYNC_ATTRIBUTE((always_inline)) void operator++(int)
    {
      (void)operator++();
    }

    COASYNC_ATTRIBUTE((nodiscard, always_inline)) reference operator*() const noexcept
    {
      return static_cast<reference>(__promise_->_M_value.get());
    }

  private:
    friend generator;

    COASYNC_ATTRIBUTE((always_inline)) explicit iterator(__promise_base* __promise, std::coroutine_handle<> __coro) noexcept
      : __promise_(__promise)
      , __coro_(__coro)
    {}

    __promise_base* __promise_;
    std::coroutine_handle<> __coro_;
  };

  COASYNC_ATTRIBUTE((nodiscard, always_inline)) iterator begin()
  {
    assert(__coro_);
    assert(!__started_);
    __started_ = true;
    __coro_.resume();
    return iterator{__promise_, __coro_};
  }

  COASYNC_ATTRIBUTE((maybe_unused, always_inline)) std::default_sentinel_t end() noexcept
  {
    return {};
  }

private:
  template<typename _Generator, typename _ByteAllocator, bool _ExplicitAllocator>
  friend struct __generator_promise;

  template<typename _Promise>
  COASYNC_ATTRIBUTE((always_inline)) explicit generator(std::coroutine_handle<_Promise> __coro) noexcept
    : __promise_(std::addressof(__coro.promise()))
    , __coro_(__coro)
  {}

public: // to get around access restrictions for __yield_sequence_awaitable
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) std::coroutine_handle<> __get_coro() noexcept
  {
    return __coro_;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) __promise_base* __get_promise() noexcept
  {
    return __promise_;
  }

private:
  __promise_base* __promise_;
  std::coroutine_handle<> __coro_;
  bool __started_ = false;
};

} // namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
namespace std
{
// Type-erased allocator with default allocator behaviour.
template<typename _Ref, typename _Value, typename... _Args>
struct coroutine_traits<coasync::generator<_Ref, _Value>, _Args...>
{
  using promise_type = coasync::__generator_promise<coasync::generator<_Ref, _Value>, std::allocator<std::byte>>;
};

// Type-erased allocator with std::allocator_arg parameter
template<typename _Ref, typename _Value, typename _Alloc, typename... _Args>
struct coroutine_traits<coasync::generator<_Ref, _Value>, std::allocator_arg_t, _Alloc, _Args...>
{
private:
  using __byte_allocator = coasync::__byte_allocator_t<_Alloc>;
public:
  using promise_type = coasync::__generator_promise<coasync::generator<_Ref, _Value>, __byte_allocator, true /*explicit Allocator*/>;
};

// Type-erased allocator with std::allocator_arg parameter (non-static member functions)
template<typename _Ref, typename _Value, typename _This, typename _Alloc, typename... _Args>
struct coroutine_traits<coasync::generator<_Ref, _Value>, _This, std::allocator_arg_t, _Alloc, _Args...>
{
private:
  using __byte_allocator = coasync::__byte_allocator_t<_Alloc>;
public:
  using promise_type = coasync::__generator_promise<coasync::generator<_Ref, _Value>, __byte_allocator,  true /*explicit Allocator*/>;
};

// Generator with specified allocator type
template<typename _Ref, typename _Value, typename _Alloc, typename... _Args>
struct coroutine_traits<coasync::generator<_Ref, _Value, _Alloc>, _Args...>
{
  using __byte_allocator = coasync::__byte_allocator_t<_Alloc>;
public:
  using promise_type = coasync::__generator_promise<coasync::generator<_Ref, _Value, _Alloc>, __byte_allocator>;
};

}
#if __has_include(<ranges>) && defined(__cpp_lib_ranges)
namespace std
{
namespace ranges
{
template <typename _T, typename _U, typename _Alloc>
constexpr inline bool enable_view<coasync::generator<_T, _U, _Alloc>> = true;
} // namespace ranges
}
#endif
#endif

#endif // COASYNC_GENERATOR_INCLUDED
