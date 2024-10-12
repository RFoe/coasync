#ifndef COASYNC_CANCELLATION_ERROR_INCLUDED
#define COASYNC_CANCELLATION_ERROR_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "detail/config.hpp"
#include <system_error>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
enum class COASYNC_ATTRIBUTE((nodiscard)) cancellation_errc
{
  cancellation_requested = 1,
  no_frame_registered,
};
}
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) std
{
/// Specialization that allows `cancellation_errc` to convert to `error_code`.
template<>
struct is_error_code_enum<coasync::cancellation_errc> : public std::true_type { };
}
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
/// Points to a statically-allocated object derived from error_category.
COASYNC_ATTRIBUTE((nodiscard, __gnu__::__const__, always_inline))
const std::error_category& cancellation_category() noexcept
{
struct COASYNC_ATTRIBUTE((nodiscard)) __category final: public std::error_category
  {
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
#if defined(__cpp_constexpr) && __cplusplus >=201907L
	constexpr
#endif
		virtual const char* name() const noexcept
  {
    return "cancellation";
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
#if defined(__cpp_constexpr) && __cplusplus >=201907L
	constexpr
#endif
	virtual std::string message(int errc) const override
  {
    switch (errc)
      {
      case static_cast<int>(cancellation_errc::cancellation_requested):
        COASYNC_ATTRIBUTE((likely))
        //TODO
        return "cancellation_requested at waiting site";
      case static_cast<int>(cancellation_errc::no_frame_registered):
        COASYNC_ATTRIBUTE((unlikely))
        //TODO
        return "no_frame_registered at cancelling site";
      default:
        COASYNC_ATTRIBUTE((unlikely))
        return "unknown";
      }
  }
  };
  static struct __category cat;
  return cat;
}

/// Overload of make_error_code for `cancellation_errc`.
COASYNC_ATTRIBUTE((nodiscard, always_inline))
std::error_code make_error_code(cancellation_errc __errc) noexcept
{
  return std::error_code(static_cast<int>(__errc), cancellation_category());
}

/// Overload of make_error_condition for `cancellation_errc`.
COASYNC_ATTRIBUTE((nodiscard, always_inline))
std::error_condition make_error_condition(cancellation_errc __errc) noexcept
{
  return std::error_condition(static_cast<int>(__errc), cancellation_category());
}

class COASYNC_ATTRIBUTE((nodiscard)) cancellation_error : public std::logic_error
{
public:
  COASYNC_ATTRIBUTE((always_inline)) explicit
  cancellation_error(cancellation_errc __errc)
    : cancellation_error(std::make_error_code(std::errc(static_cast<int>(__errc))))
  {}

  COASYNC_ATTRIBUTE((always_inline))
#if defined(__cpp_constexpr) && __cplusplus >=201907L
 /*constexpr*/
#endif
	virtual ~cancellation_error() noexcept {}

  COASYNC_ATTRIBUTE((nodiscard, always_inline))
#if defined(__cpp_constexpr) && __cplusplus >=201907L
 /*constexpr*/
#endif
  virtual const char* what() const noexcept
  {
  	auto errc = static_cast<std::underlying_type_t<std::errc>>(_M_code.value());
    switch (errc)
      {
      case static_cast<int>(cancellation_errc::cancellation_requested):
        COASYNC_ATTRIBUTE((likely))
        //TODO
        return "cancellation_requested at waiting site";
      case static_cast<int>(cancellation_errc::no_frame_registered):
        COASYNC_ATTRIBUTE((unlikely))
        //TODO
        return "no_frame_registered at cancelling site";
      default:
        COASYNC_ATTRIBUTE((unlikely))
        return "unknown";
      }
  }

  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  const std::error_code& code() const noexcept
  {
    return _M_code;
  }

private:
  COASYNC_ATTRIBUTE((always_inline)) explicit
  cancellation_error(std::error_code __ec)
    : logic_error("std::cancellation_error: " + __ec.message())
    , _M_code(__ec)
  { }

  COASYNC_ATTRIBUTE((no_unique_address)) std::error_code 			_M_code;
};
}

#endif
