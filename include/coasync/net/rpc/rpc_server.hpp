#ifndef COASYNC_RPC_SERVER_INCLUDED
#define COASYNC_RPC_SERVER_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../../async_fn.hpp"
#include "../../co_spawn.hpp"
#include "../../detail/get_context.hpp"
#include "../serde_stream.hpp"
#include "../protocol.hpp"
#include "../endpoint.hpp"
#include <unordered_map>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
template <typename protocol_type, typename execution_context>
struct __basic_acceptor;
struct tcp;
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) rpc
{
/// a RPC library for C++, providing both a client and server implementation.
/// It is built using modern C++20 coroutines, and as such, requires a recent
/// compiler.
template <typename execution_context> struct __rpc_server
{
/// aims to save the protocol implementers the hassle of implementing their own
/// remote procedure call api and the application programmers the hassle of jumping
/// through hoops just to expose their services using multiple protocols and transports.
/// rpcserver/client comes with the implementations of popular transport, protocol[tcp]
/// and interface.
private:
  typedef std::unordered_map<std::string, async_fn<awaitable<void>(serde_stream&)>> service_table;
public:
	typedef tcp 																			protocol_type;
	typedef std::string                               service_key;
  typedef __serde_stream<execution_context> 				serde_stream;
  typedef __basic_acceptor<tcp, execution_context> 	acceptor;
  template <typename F> requires detail::function_traits<F>::value
  COASYNC_ATTRIBUTE((always_inline))
  void bind(std::string __str, F __fn)
  {
    _M_services.emplace(std::move(__str), [__f = std::move(__fn)]
#if __cplusplus >= 202207L
                        COASYNC_ATTRIBUTE((nodiscard))
#endif
                        (serde_stream& __stream) mutable -> awaitable<void>
    {
      COASYNC_ATTRIBUTE((gnu::uninitialized)) typename detail::function_traits<F>::tuple args;
      try
        {
          co_await __stream.deserialize(args);
          std::this_thread::sleep_for(std::chrono::seconds(2));
          co_await __stream.serialize(std::apply(__f, args));
        }
      catch(std::system_error& err)
        {
          /// The peer end actively disconnects when sending and receiving packets.
          if(err.code().value() == static_cast<int>(std::errc::connection_aborted)) COASYNC_ATTRIBUTE((likely))
            co_return;
          std::rethrow_exception(std::make_exception_ptr(std::move(err)));
        }
    });
  }
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<void> start()
  {
    while(true)
      {
        auto socket = co_await _M_acceptor.accept();
        co_spawn(co_await detail::get_context(), []
#if __cplusplus >= 202207L
                 COASYNC_ATTRIBUTE((nodiscard))
#endif
                 (serde_stream __stream, service_table* __table) -> awaitable<void>
        {
          while(true)
            {
              COASYNC_ATTRIBUTE((gnu::uninitialized)) std::string __fn;
              COASYNC_ATTRIBUTE((assume(__table != nullptr)));
              try
                {
                  co_await __stream.deserialize(__fn);
                }
              catch(std::system_error& err)
                {
                  if(err.code().value() == static_cast<int>(std::errc::connection_aborted)) COASYNC_ATTRIBUTE((likely))
                    break;
                  std::rethrow_exception(std::make_exception_ptr(std::move(err)));
                }
              co_await (* __table)[__fn](__stream);
            }
        }(serde_stream(std::move(socket)), &_M_services), use_detach);
      }
  }
  COASYNC_ATTRIBUTE((always_inline))
  explicit __rpc_server(acceptor ac): _M_acceptor(std::move(ac)) {}
private:
  service_table _M_services;
  acceptor 			_M_acceptor;
};
typedef __rpc_server<execution_context> rpc_server;
}
}
}

#endif
