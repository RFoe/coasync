#ifndef COASYNC_RPC_CLIENT_INCLUDED
#define COASYNC_RPC_CLIENT_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../serde_stream.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) rpc
{
template <typename execution_context> struct __rpc_client
{
private:
  typedef __serde_stream<execution_context> 			serde_stream;
  typedef __basic_socket<tcp, execution_context> 	socket;
public:
  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit __rpc_client(socket s) noexcept: _M_stream(std::move(s)) {}
  template <typename R, typename... Args>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<R> call(std::string const& __fn, Args&& ... __args)
  {
    co_await _M_stream.serialize(__fn);
    std::tuple<Args...> args { std::forward<Args>(__args)... };
    co_await _M_stream.serialize(args);
    COASYNC_ATTRIBUTE((gnu::uninitialized)) union _Storage
    {
      R _M_value;
    } __storage;
		co_await _M_stream.deserialize((R &)__storage._M_value);
		co_return std::move(__storage._M_value);
  }
private:
  serde_stream _M_stream;
};
typedef __rpc_client<execution_context> rpc_client;
}
}
}
#endif
