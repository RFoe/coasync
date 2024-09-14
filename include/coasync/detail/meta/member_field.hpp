#ifndef COASYNC_MERBER_FIELD_INCLUDED
#define COASYNC_MERBER_FIELD_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../config.hpp"

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
/// provides us the meta info of the object via meta-object types.

#define _COASYNC_MEMBER_FIELD(_S, _Pt) coasync::detail::member_field(&_S::_Pt, #_Pt)
#define _COASYNC_ARGS_ENWRAP_0(...)
#define _COASYNC_ARGS_ENWRAP_1(_S, _1) \
	_COASYNC_MEMBER_FIELD(_S, _1)
#define _COASYNC_ARGS_ENWRAP_2(_S, _1, _2) \
	_COASYNC_MEMBER_FIELD(_S, _1), _COASYNC_MEMBER_FIELD(_S, _2)
#define _COASYNC_ARGS_ENWRAP_3(_S, _1, _2, _3) \
	_COASYNC_MEMBER_FIELD(_S, _1), _COASYNC_MEMBER_FIELD(_S, _2), _COASYNC_MEMBER_FIELD(_S, _3)
#define _COASYNC_ARGS_ENWRAP_4(_S, _1, _2, _3, _4) \
	_COASYNC_MEMBER_FIELD(_S, _1), _COASYNC_MEMBER_FIELD(_S, _)2, _COASYNC_MEMBER_FIELD(_S, _3), _COASYNC_MEMBER_FIELD(_S, _4)
#define _COASYNC_ARGS_ENWRAP_5(_S, _1, _2, _3, _4, _5) \
	_COASYNC_MEMBER_FIELD(_S, _1), _COASYNC_MEMBER_FIELD(_S, _2), _COASYNC_MEMBER_FIELD(_S, _3), _COASYNC_MEMBER_FIELD(_S, _4), _COASYNC_MEMBER_FIELD(_S, _5)
#define COASYNC_ARGS_ENWRAP(_S, ...) _COASYNC_ARGS_ENWRAP_INTERNAL(_S, COASYNC_COUNT(__VA_ARGS__), __VA_ARGS__)
#define _COASYNC_ARGS_ENWRAP_INTERNAL(_S, _N, ...) COASYNC_CONCAT(_COASYNC_ARGS_ENWRAP_, _N)(_S, __VA_ARGS__)

#include <tuple>

#define struct_meta(_S, ...) \
template <> struct coasync::detail::meta<_S> { \
	static constexpr auto fields = std::make_tuple(COASYNC_ARGS_ENWRAP(_S, __VA_ARGS__)); \
};

#endif
