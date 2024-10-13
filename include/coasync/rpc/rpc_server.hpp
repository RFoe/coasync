#ifndef COASYNC_RPC_SERVER_INCLUDED
#define COASYNC_RPC_SERVER_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "rpc_control_variable.hpp"
#include "../async_fn.hpp"
#include "../co_spawn.hpp"
#include "../detail/get_context.hpp"
#include "../detail/object_deduce.hpp"
#include "../net/serde_stream.hpp"
#include "../net/protocol.hpp"
#include "../net/endpoint.hpp"

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
template <typename execution_context> struct COASYNC_ATTRIBUTE((nodiscard)) basic_rpc_server
{
/// aims to save the protocol implementers the hassle of implementing their own
/// remote procedure call api and the application programmers the hassle of jumping
/// through hoops just to expose their services using multiple protocols and transports.
/// rpcserver/client comes with the implementations of popular transport, protocol[tcp]
/// and interface.
private:
  typedef std::unordered_map<std::string,
          async_fn<awaitable<void>(serde_stream&)>>
          service_table;
  typedef basic_serde_stream<execution_context> 				serde_stream;
  template <typename Callback>
  struct COASYNC_ATTRIBUTE((nodiscard)) service_invocable
  {
    template <typename OtherCallback>
    requires (not std::is_same_v<std::remove_cvref_t<Callback>, service_invocable>)
    COASYNC_ATTRIBUTE((always_inline))
    constexpr explicit service_invocable(OtherCallback&& cb)
    noexcept(std::is_nothrow_constructible_v<Callback>)
    : _M_callback(std::forward<OtherCallback>(cb))
  {}
  COASYNC_ATTRIBUTE((always_inline)) constexpr service_invocable& operator=(service_invocable const&)
  noexcept(std::is_nothrow_copy_assignable_v<Callback>) = default;
      COASYNC_ATTRIBUTE((always_inline)) constexpr service_invocable(service_invocable const&)
      noexcept(std::is_nothrow_move_assignable_v<Callback>) = default;
      COASYNC_ATTRIBUTE((always_inline)) constexpr service_invocable(service_invocable&&) noexcept = default;
      COASYNC_ATTRIBUTE((always_inline)) constexpr service_invocable& operator=(service_invocable&&) noexcept = default;
      COASYNC_ATTRIBUTE((always_inline)) ~ service_invocable() noexcept = default;

      COASYNC_ATTRIBUTE((nodiscard)) awaitable<void> operator()(serde_stream& __stream) noexcept(false)
  {
    COASYNC_ATTRIBUTE((gnu::uninitialized)) typename detail::function_traits<Callback>::tuple arguments;
    try
      {
        co_await __stream.deserialize(arguments);
        if constexpr(std::is_void_v<typename detail::function_traits<Callback>::result>)
          {
            (void) std::apply(static_cast<Callback&>(_M_callback), arguments);
            COASYNC_ATTRIBUTE((gnu::uninitialized)) detail::object_deduce_t<void> void_response;
            co_await __stream.serialize(void_response);
          }
        else
          {
            co_await __stream.serialize(std::apply(static_cast<Callback&>(_M_callback), arguments));
          }
      }
    catch(std::system_error& err)
      {
        /// The peer end actively disconnects when sending and receiving packets.
        if(err.code() == std::make_error_code(std::errc::connection_aborted)
            or err.code() == std::make_error_code(std::errc::connection_reset)) COASYNC_ATTRIBUTE((likely))
          co_return;
        std::rethrow_exception(std::make_exception_ptr(std::move(err)));
      }
    catch(...)
      {
        std::rethrow_exception(std::current_exception());
      }
  }
  private:
  COASYNC_ATTRIBUTE((no_unique_address)) Callback _M_callback;
             };
#if defined(__cpp_deduction_guides)
  template <typename OtherCallback>
  service_invocable(OtherCallback&&)
  -> service_invocable<std::decay_t<OtherCallback>>;
#endif

  struct COASYNC_ATTRIBUTE((nodiscard)) acceptance_invocable
  {
    COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit acceptance_invocable() noexcept {}
  constexpr acceptance_invocable& operator=(acceptance_invocable const&) = delete;
      constexpr acceptance_invocable& operator=(acceptance_invocable&&) = delete;
      COASYNC_ATTRIBUTE((always_inline)) ~ acceptance_invocable() noexcept = default;

      COASYNC_ATTRIBUTE((nodiscard))
      awaitable<void> operator()(serde_stream __stream, service_table* __table) const&& noexcept(false)
      COASYNC_ATTRIBUTE((gnu::nonnull))
  {
//    COASYNC_ATTRIBUTE((gnu::uninitialized)) bool _S_close;
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
            if(err.code() == std::make_error_code(std::errc::connection_aborted)
                or err.code() == std::make_error_code(std::errc::connection_reset))
              COASYNC_ATTRIBUTE((likely))
              co_return;
            std::rethrow_exception(std::make_exception_ptr(std::move(err)));
          }
        catch(...)
          {
            std::rethrow_exception(std::current_exception());
          }
        if(not __table->contains(__fn)) COASYNC_ATTRIBUTE((unlikely))
          throw std::runtime_error(std::string("rpc_server: could not find service ") + __fn);
        co_await (* __table)[__fn](__stream);
      }
  }
  };

public:
  typedef tcp 																			protocol_type;
  typedef std::string                               service_key;
  typedef __basic_acceptor<tcp, execution_context> 	acceptor;

  template <typename F> requires detail::function_traits<F>::value
  COASYNC_ATTRIBUTE((always_inline))
  void bind(std::string __str, F __fn) noexcept(false)
  {
    _M_services.emplace(std::move(__str), service_invocable (std::move(__fn)));
  }
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<void> start() noexcept(false)
  {
    while(true)
      {
        auto socket = co_await _M_acceptor.accept();
        co_spawn(
          co_await detail::get_context(),
          acceptance_invocable()(serde_stream(std::move(socket)), std::addressof(_M_services)),
          use_detach);
      }
  }
  COASYNC_ATTRIBUTE((always_inline))
  explicit basic_rpc_server(acceptor ac)
    : _M_acceptor(std::move(ac))
  {
    COASYNC_ASSERT((_M_acceptor.is_open()));
  }
  constexpr basic_rpc_server& operator=(basic_rpc_server const&) = delete;
  constexpr basic_rpc_server(basic_rpc_server const&) = delete;
  COASYNC_ATTRIBUTE((always_inline)) basic_rpc_server(basic_rpc_server&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) basic_rpc_server& operator=(basic_rpc_server&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) ~ basic_rpc_server() noexcept = default;

  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  basic_rpc_server::acceptor& get_acceptor() noexcept
  {
    return _M_acceptor;
  }
private:
  service_table _M_services;
  acceptor 			_M_acceptor;
};
typedef basic_rpc_server<execution_context> rpc_server;
}
}
}

#endif
