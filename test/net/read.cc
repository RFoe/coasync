#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/co_spawn.hpp"
#include "../include/coasync/net/endpoint.hpp"
#include "../include/coasync/net/acceptor.hpp"
#include "../include/coasync/net/receive.hpp"
#include "../include/coasync/net/protocol.hpp"
#include "../include/coasync/functional.hpp"
#include "../include/coasync/this_coro.hpp"
#include <iostream>
using namespace coasync;
awaitable<void> connetion(net::tcp::socket client) noexcept
{
  std::puts("hello client");
  while(true)
      std::cout << co_await net::receive_at_least(client, 8) << "\n";
}
awaitable<void> acceptance() noexcept
{
  net::tcp::acceptor acceptor
  {
    co_await this_coro::context,
    net::tcp::endpoint(net::address_v4::loopback(), 10086)
  };
  while(true)
      co_spawn(co_await this_coro::context, connetion(co_await acceptor.accept()), use_detach);
}
int main()
{
  execution_context context {0};
  co_spawn(context, acceptance(), use_detach);
  context.loop();
}
