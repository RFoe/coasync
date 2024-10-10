#include "../../include/coasync/execution_context.hpp"
#include "../../include/coasync/co_spawn.hpp"
#include "../../include/coasync/this_coro.hpp"
#include "../../include/coasync/net/endpoint.hpp"
#include "../../include/coasync/net/acceptor.hpp"
#include "../../include/coasync/net/protocol.hpp"
#include "../../include/coasync/net/serde_stream.hpp"

#include <iostream>

using namespace coasync;

struct example
{
  int ivalue;
  double fvalue;
  std::vector<int> ivector;
};
struct_meta(example, ivalue, fvalue, ivector);

awaitable<void> deserial(net::tcp::socket socket)
{
	std::puts("new client");
  net::serde_stream s { std::move(socket) };
  [[gnu::uninitialized]] example ex;
  co_await s.deserialize(ex);
  std::cout << "ivalue: " << ex.ivalue << "\n";
  std::cout << "fvalue: " << ex.fvalue << "\n";
  for(auto v: ex.ivector)
    std::cout << "ivector element: " << v << "\n";
}

awaitable<void> acceptance() noexcept
{
  net::tcp::acceptor acceptor
  {
    co_await this_coro::context,
    net::tcp::endpoint(net::address_v4::loopback(), 10086)
  };
  while(true)
    co_spawn(co_await this_coro::context, deserial(co_await acceptor.accept()), use_detach);
}

int main()
{
  execution_context context {concurrency_arg(2)};
  co_spawn(context, acceptance(), use_detach);
  context.loop();
}
