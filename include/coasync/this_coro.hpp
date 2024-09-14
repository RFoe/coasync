#ifndef COASYNC_THIS_CORO_INCLUDED
#define COASYNC_THIS_CORO_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "detail/get_context.hpp"
#include "detail/get_id.hpp"
#include "detail/get_stop_token.hpp"
#include "detail/get_frame.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace this_coro {
/// Awaitable object that returns the execution_context reference of the current coroutine.
inline constexpr detail::get_context 		context {};
/// Awaitable object that returns the coroutine-id of the current coroutine.
inline constexpr detail::get_id       	id {};
/// Awaitable object that returns the coroutine-handle of the current coroutine.
inline constexpr detail::get_frame 			frame {};
/// Awaitable object that returns the stop_token of the current coroutine.
inline detail::get_stop_token         	stop_token {};
};
}
#endif
