#ifndef COASYNIO_MANUAL_LIFETIME_INCLUDED
#define COASYNIO_MANUAL_LIFETIME_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "config.hpp"
#include <new>
#include <utility>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
/// for lazy initialize using union wrapper
/// the uninitialized storage designated by union / pointer
template <typename T>
struct manual_lifetime
{
  COASYNC_ATTRIBUTE((always_inline))
	explicit manual_lifetime() noexcept {}
  COASYNC_ATTRIBUTE((always_inline))
  ~manual_lifetime() {}
  template <typename... Args>
  COASYNC_ATTRIBUTE((maybe_unused, always_inline))
	T& construct(Args&& ... args)
  noexcept(std::is_nothrow_constructible_v<T, Args...>)
  {
    return *::new (static_cast<void*>(std::addressof(_M_value))) T(std::forward<Args>(args)...);
  }
  COASYNC_ATTRIBUTE((always_inline))
	void destruct() noexcept(std::is_nothrow_destructible_v<T>)
  {
    _M_value.~T();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
	T& get()& noexcept
  {
    return _M_value;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
	T&& get()&& noexcept
  {
    return static_cast <T &&> (_M_value);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
	const T& get() const& noexcept
  {
    return _M_value;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
	const T&& get() const&& noexcept
  {
    return static_cast <const T&&> (_M_value);
  }
private:
/// union: no initialization is performed and the beginning of its lifetime is
/// sequenced after the value computation of the left and right operands and before the assignment
  union
  {
    COASYNC_ATTRIBUTE((no_unique_address)) std::remove_const_t<T> _M_value;
  };
};
template <typename T>
struct manual_lifetime<T&>
{
  COASYNC_ATTRIBUTE((maybe_unused, always_inline))
	T& construct(T& value) noexcept
  {
    _M_value = std::addressof(value);
    return value;
  }
  COASYNC_ATTRIBUTE((always_inline))
	void destruct() noexcept {}
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
	T& get() const noexcept
  {
    return *_M_value;
  }
private:
  T* _M_value;
};
template <typename T>
struct manual_lifetime <T&&>
{
  COASYNC_ATTRIBUTE((maybe_unused, always_inline))
	T&& construct(T&& value) noexcept
  {
    _M_value = std::addressof(value);
    return static_cast <T &&> (value);
  }
  COASYNC_ATTRIBUTE((always_inline))
	void destruct() noexcept {}
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
	T&& get() const noexcept
  {
    return static_cast <T &&> (*_M_value);
  }
private:
  T* _M_value;
};
template <> struct manual_lifetime<void> {
	COASYNC_ATTRIBUTE((always_inline))
	constexpr void construct() const noexcept {}
	COASYNC_ATTRIBUTE((always_inline))
	constexpr void destruct() const noexcept {}
	COASYNC_ATTRIBUTE((always_inline))
	constexpr void get() const noexcept {}
};
}
}
#endif
