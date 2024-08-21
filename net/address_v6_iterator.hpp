#ifndef __COASYNC_ADDRESS_V4_ITERATOR_INCLUDED
#define __COASYNC_ADDRESS_V4_ITERATOR_INCLUDED
#include "address_v6.hpp"
#include <ranges>
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
template <typename> struct basic_address_iterator;
template <> struct basic_address_iterator<address_v6>
{
  typedef address_v6 						value_type;
  typedef std::ptrdiff_t 				difference_type;
  typedef address_v6* 					pointer;
  typedef address_v6 const* 		const_pointer;
  typedef address_v6& 					reference;
  typedef address_v6 const& 		const_reference;
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
  constexpr basic_address_iterator(address_v6 const& a) noexcept : M_address(a)
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
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  basic_address_iterator& COASYNC_API operator++() noexcept
  {
    for (unsigned char& reference : M_address.M_bytes | std::views::reverse)
      {
        if (reference < std::numeric_limits<unsigned char>::max()) COASYNC_ATTRIBUTE((likely))
          {
            reference ++;
            break;
          }
        reference = std::numeric_limits<unsigned char>::min();
      }
    return (*this);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  basic_address_iterator COASYNC_API operator++(int) noexcept
  {
    basic_address_iterator tmp = *this;
    (void) operator++();
    return tmp;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  basic_address_iterator& COASYNC_API operator--() noexcept
  {
    for (unsigned char& reference : M_address.M_bytes | std::views::reverse)
      {
        if (reference > std::numeric_limits<unsigned char>::min()) COASYNC_ATTRIBUTE((likely))
          {
            reference --;
            break;
          }
        reference = std::numeric_limits<unsigned char>::max();
      }
    return (*this);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
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
  address_v6 M_address;
};
typedef basic_address_iterator<address_v6> address_v6_iterator;
template <typename> struct
  basic_address_range;
template <> struct basic_address_range<address_v6>
  : public std::ranges::view_interface<basic_address_range<address_v6>>
{
  typedef basic_address_iterator<address_v6> iterator;
  typedef typename basic_address_iterator<address_v6>::value_type
  value_type;
  typedef typename basic_address_iterator<address_v6>::difference_type
  difference_type;
  typedef typename basic_address_iterator<address_v6>::pointer
  pointer;
  typedef typename basic_address_iterator<address_v6>::const_pointer
  const_pointer;
  typedef typename basic_address_iterator<address_v6>::reference
  reference;
  typedef typename basic_address_iterator<address_v6>::const_reference
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
    address_v6 const& first, address_v6 const& last) noexcept
    : M_begin(first)
    , M_end(last)
  {
  }
  COASYNC_ATTRIBUTE((always_inline))
  ~ basic_address_range() noexcept
  {
  };
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  basic_address_iterator<address_v6> COASYNC_API begin() const noexcept
  {
    return M_begin;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  basic_address_iterator<address_v6> COASYNC_API end() const noexcept
  {
    return M_end;
  }
private:
  basic_address_iterator<address_v6> 	M_begin {};
  basic_address_iterator<address_v6> 	M_end {};
};
typedef basic_address_range<address_v6> address_v6_range;
}
}
#endif
