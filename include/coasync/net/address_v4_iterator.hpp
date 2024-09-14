#ifndef __COASYNC_ADDRESS_V4_ITERATOR_INCLUDED
#define __COASYNC_ADDRESS_V4_ITERATOR_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "address_v4.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
template <typename> struct basic_address_iterator;
template <> struct basic_address_iterator<address_v4>
{
  typedef address_v4 				value_type;
  typedef std::ptrdiff_t 			difference_type;
  typedef address_v4* 			pointer;
  typedef address_v4 const* 		const_pointer;
  typedef address_v4& 			reference;
  typedef address_v4 const& 		const_reference;
  typedef std::input_iterator_tag iterator_category;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit basic_address_iterator() noexcept
  {
  };
  COASYNC_ATTRIBUTE((always_inline))
  constexpr basic_address_iterator(basic_address_iterator const&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr basic_address_iterator& operator=(basic_address_iterator const&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr basic_address_iterator(basic_address_iterator&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr basic_address_iterator& operator=(basic_address_iterator&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr basic_address_iterator(address_v4 const& a) noexcept : M_address(a)
  {
  }
  COASYNC_ATTRIBUTE((always_inline))
  ~ basic_address_iterator() noexcept
  {
  };
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr reference COASYNC_API operator*() noexcept
  {
    return M_address;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr pointer COASYNC_API operator->() noexcept
  {
    return std::addressof(M_address);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr const_reference COASYNC_API operator*() const noexcept
  {
    return M_address;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  constexpr const_pointer COASYNC_API operator->() const noexcept
  {
    return std::addressof(M_address);
  }
  COASYNC_ATTRIBUTE((maybe_unused, always_inline))
  basic_address_iterator& COASYNC_API operator++() noexcept
  {
    M_address = value_type((M_address.to_uint() + 1) &
                           0xFFFFFFFF);
    return *this;
  }
  COASYNC_ATTRIBUTE((maybe_unused, always_inline))
  basic_address_iterator COASYNC_API operator++(int) noexcept
  {
    basic_address_iterator tmp = *this;
    (void) operator++();
    return tmp;
  }
  COASYNC_ATTRIBUTE((maybe_unused, always_inline))
  basic_address_iterator& COASYNC_API operator--()
  noexcept
  {
    M_address = value_type((M_address.to_uint() - 1) &
                           0xFFFFFFFF);
    return *this;
  }
  COASYNC_ATTRIBUTE((maybe_unused, always_inline))
  basic_address_iterator COASYNC_API operator--(int) noexcept
  {
    basic_address_iterator tmp = *this;
    (void) operator--();
    return tmp;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  bool COASYNC_API operator==(const basic_address_iterator& rhs) const noexcept
  {
    return M_address == rhs.M_address;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
	bool COASYNC_API operator!=(const basic_address_iterator& rhs) const noexcept
  {
    return M_address != rhs.M_address;
  }
private:
  address_v4 M_address;
};
typedef basic_address_iterator<address_v4> address_v4_iterator;
template <typename> struct
  basic_address_range;
template <> struct basic_address_range<address_v4>
  : public std::ranges::view_interface<basic_address_range<address_v4>>
{
  typedef basic_address_iterator<address_v4> iterator;
  typedef typename basic_address_iterator<address_v4>::value_type
  value_type;
  typedef typename basic_address_iterator<address_v4>::difference_type
  difference_type;
  typedef typename basic_address_iterator<address_v4>::pointer
  pointer;
  typedef typename basic_address_iterator<address_v4>::const_pointer
  const_pointer;
  typedef typename basic_address_iterator<address_v4>::reference
  reference;
  typedef typename basic_address_iterator<address_v4>::const_reference
  const_reference;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr basic_address_range(basic_address_range const&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr basic_address_range& operator=(basic_address_range const&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr basic_address_range(basic_address_range&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr basic_address_range& operator=(basic_address_range&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit basic_address_range(
    address_v4 const& first, address_v4 const& last) noexcept
    : M_begin(first)
    , M_end(last)
  {
  }
  COASYNC_ATTRIBUTE((always_inline))
  ~ basic_address_range() noexcept
  {
  };
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  basic_address_iterator<address_v4> COASYNC_API begin() const noexcept
  {
    return M_begin;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  basic_address_iterator<address_v4> COASYNC_API end() const noexcept
  {
    return M_end;
  }
private:
  basic_address_iterator<address_v4> 	M_begin {};
  basic_address_iterator<address_v4> 	M_end {};
};
typedef basic_address_range<address_v4> address_v4_range;
}
}
#endif
