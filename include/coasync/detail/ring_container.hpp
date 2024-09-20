/**
 * std::queue<T, Container>
 * Container 	- 	The type of the underlying container to use to store the elements.
 * The container must satisfy the requirements of SequenceContainer. Additionally,
 * it must provide the following functions with the usual semantics:
 **/
 
/// A SequenceContainer is a Container that stores objects of the same type in a linear arrangement.

#ifndef COASYNC_RING_CONTAINER_INCLUDED
#define COASYNC_RING_CONTAINER_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "config.hpp"

#include <array>

template <typename T, std::size_t N> requires (N > 0)
struct [[nodiscard]] ring_container: public std::array<T, N>
{
private:
	template <typename... Args>
	static constexpr bool is_self = sizeof...(Args) == 1 and (std::is_same_v<std::decay_t<Args>, ring_container> || ...);
public:
  typedef std::array<T, N>::reference reference;
  typedef std::array<T, N>::const_reference const_reference;
  typedef std::array<T, N>::value_type value_type;
  typedef std::array<T, N>::reverse_iterator reverse_iterator;
  typedef std::array<T, N>::const_reverse_iterator const_reverse_iterator;
  typedef std::array<T, N>::iterator iterator;
  typedef std::array<T, N>::const_iterator const_iterator;
  typedef std::array<T, N>::pointer pointer;
  typedef std::array<T, N>::const_pointer const_pointer;
  typedef std::array<T, N>::difference_type difference_type;
  typedef std::array<T, N>::size_type size_type;
  using std::array<T, N>::operator[];

	//  std::array supports assignment from a braced-init-list, but not from an std::initializer_list.
  template <typename... CtorArgs> requires (not is_self<CtorArgs ...>)
	COASYNC_ATTRIBUTE((always_inline))
  constexpr ring_container(CtorArgs&& ... ctorargs)
  noexcept(std::is_nothrow_constructible_v<std::array<T, N>, CtorArgs ...>)
    : std::array<T, N>(std::forward<CtorArgs>(ctorargs) ...)
    , _M_head(), _M_tail() {}
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) constexpr reference front() noexcept
  {
    return (*this)[_M_head];
  }
  constexpr ring_container(ring_container const&) noexcept = default;
  constexpr ring_container& operator=(ring_container const&) noexcept = default;
  constexpr ring_container(ring_container &&) noexcept = default;
  constexpr ring_container& operator=(ring_container &&) noexcept = default;
  ~ ring_container() noexcept = default;
	constexpr void swap(ring_container& other) noexcept {
		std::array<T, N>::swap(other);
		std::swap(_M_head, other._M_head);
		std::swap(_M_tail, other._M_tail);
	}

  COASYNC_ATTRIBUTE((nodiscard, __gnu__::__const__, always_inline)) constexpr const_reference front() const noexcept
  {
    return (*this)[_M_head];
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) constexpr reference back() noexcept
  {
    return (*this)[_M_tail];
  }
  COASYNC_ATTRIBUTE((nodiscard, __gnu__::__const__, always_inline)) constexpr const_reference back() const noexcept
  {
    return (*this)[_M_tail];
  }
  template <typename U>
  COASYNC_ATTRIBUTE((always_inline)) void push_back(U&& value) noexcept(std::is_nothrow_constructible_v<T, U>)
	{
		(void) emplace_back(static_cast<U &&>(value));
	}
  template <typename... CtorArgs>
  COASYNC_ATTRIBUTE((maybe_unused, always_inline)) reference emplace_back(CtorArgs&& ... ctorargs) noexcept(std::is_nothrow_constructible_v<T, CtorArgs ...>)
  {
  	static_assert(std::is_constructible_v<T, CtorArgs ...>);
		[[gnu::uninitialized]] T* pointer;
		pointer = ::new(std::addressof((*this)[_M_tail])) T{ std::forward<CtorArgs>(ctorargs) ...};
    _M_tail = (_M_tail + 1) % N;
    return * pointer;
  }
  COASYNC_ATTRIBUTE((always_inline)) void pop_front() noexcept
  {
    _M_head = (_M_head + 1) % N;
  }
  COASYNC_ATTRIBUTE((nodiscard, __gnu__::__const__, always_inline)) constexpr size_type size() const noexcept
	{
		return (_M_tail + N - _M_head) % N;
	}
  COASYNC_ATTRIBUTE((nodiscard, __gnu__::__const__, always_inline)) constexpr bool empty() const noexcept
	{
 		return _M_tail == _M_head;
	}
private:
  std::size_t _M_head {};
  std::size_t _M_tail {};
};

#endif
