#include "../include/coasync/this_coro.hpp"
#include "../include/coasync/co_spawn.hpp"
#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/net/acceptor.hpp"
#include "../include/coasync/net/endpoint.hpp"
#include "../include/coasync/net/protocol.hpp"
#include "../include/coasync/net/serde_stream.hpp"
#include "../include/coasync/detail/meta/function_traits.hpp"
#include <map>
using namespace coasync;
template <typename> struct function;
template <typename R, typename... Args>
struct function<awaitable<R>(Args ...)>
{
  struct base
  {
    virtual awaitable<R> call(Args ...) = 0;
  };
  template <typename F>
  struct impl: base
  {
    impl(F&& f): _M_f((F&&) f) {}
    F _M_f;
    virtual awaitable<R> call(Args ...args) override
    {
      std::puts("call");
      return _M_f((Args) args ...);
    }
  };
  awaitable<R> operator()(Args ...args)
  {
    std::puts("operator()");
    return _M_ptr -> call((Args) args ...);
  };
																			 function(){ }
  template <typename F>
  function(F&& f) : _M_ptr(std::make_unique<impl<F>>(((F &&)f))) {}
  std::unique_ptr<base> _M_ptr;
};

std::map<std::string, function<awaitable<void>(net::serde_stream&)>> services;

awaitable<void> deserial(net::serde_stream s)
{
  std::string f;
  co_await s.deserialize(f);
  co_await services[f](s);
}
awaitable<void> test() noexcept
{
  net::tcp::acceptor acceptor
  {
    co_await this_coro::context,
    net::tcp::endpoint(net::address_v4::loopback(), 10086)
  };
	auto add_f = [](int a, int b) -> int  { return a + b; };
 services.emplace("add", [add_f](net::serde_stream& s) ->awaitable<void> {
		std::tuple<int, int> args;
		co_await s.deserialize(args);
		co_await s.serialize(std::apply(add_f, args));
 });
 auto socket = co_await acceptor.accept();
 co_spawn(co_await this_coro::context, deserial(net::serde_stream(std::move(socket))), use_detach);
}

int main()
{
  execution_context context {0};
  co_spawn(context, test(), use_detach);
  context.loop();
}
