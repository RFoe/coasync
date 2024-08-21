#ifndef COASYNC_MERBER_FIELD_INCLUDED
#define COASYNC_MERBER_FIELD_INCLUDED
#include "../config.h"
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
/// wraps a member pointer and its name as a type-safe structure. This can be useful
/// for manipulating member pointers and extracting information about them at compile time.
template <typename> struct member_field;
template <typename Class, typename T>
struct member_field<T Class::*>
{
	/// A container for storing basic information about class members
  typedef Class class_type;
  typedef T     value_type;
 	COASYNC_ATTRIBUTE((always_inline))
	consteval member_field(T Class::* ptr, char const* name) noexcept
    : _M_ptr(ptr), _M_name(name) { }
	COASYNC_ATTRIBUTE((nodiscard, always_inline))
	constexpr T Class::* pointer() const noexcept {
		return _M_ptr;
	}
	COASYNC_ATTRIBUTE((nodiscard, always_inline))
	constexpr char const* name() const noexcept {
		return _M_name;
	}
private:
  T Class::*	_M_ptr;
  char const* _M_name;
};
template <typename Class, typename T>
member_field(T Class::*, char const*) -> member_field<T Class::*>;
template <typename Class> struct meta: std::false_type {};
}
}

#endif
