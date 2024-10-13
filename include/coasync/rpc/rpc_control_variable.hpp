#ifndef COASYNC_RPC_CONTROL_VARIABLE_INCLUDED
#define COASYNC_RPC_CONTROL_VARIABLE_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../detail/config.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) rpc
{
struct COASYNC_ATTRIBUTE((nodiscard)) rpc_control_variable
{
  COASYNC_ATTRIBUTE((always_inline))
  constexpr void disable_response() noexcept
  {
    _M_disable_response = true;
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr void close_session() noexcept
  {
    COASYNC_ATTRIBUTE((assume((_M_close_session != nullptr))));
    * _M_close_session = true;
  }
  COASYNC_ATTRIBUTE((always_inline))
  constexpr void shutdown_server() noexcept
  {
    COASYNC_ATTRIBUTE((assume((_M_close_session != nullptr))));
    * _M_shutdown_server = true;
  }
private:
	template <typename execution_context>
		friend struct basic_rpc_server;
  COASYNC_ATTRIBUTE((always_inline))
  explicit constexpr rpc_control_variable() noexcept
    : _M_disable_response(false)
  {}
  alignas(64) bool _M_disable_response;
  alignas(64) bool* _M_close_session;
  alignas(64) bool* _M_shutdown_server;
};
}
}
}
#endif
