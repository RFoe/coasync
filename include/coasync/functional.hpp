#ifndef COASYNC_FUNCTIONAL_INCLUDED
#define COASYNC_FUNCTIONAL_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "detail/service/time_service.hpp"
#include "detail/service/yield_service.hpp"
#include "detail/suspendible.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
inline constexpr detail::suspendible<detail::time_service> 	sleep_for;
inline constexpr detail::suspendible<detail::time_service> 	sleep_until;
inline constexpr detail::suspendible<detail::yield_service> yield;
}
#endif
