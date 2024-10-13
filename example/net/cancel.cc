#include "../../include/coasync/execution_context.hpp"
#include "../../include/coasync/co_spawn.hpp"
#include "../../include/coasync/net/endpoint.hpp"
#include "../../include/coasync/net/acceptor.hpp"
#include "../../include/coasync/net/receive.hpp"
#include "../../include/coasync/net/protocol.hpp"
#include "../../include/coasync/functional.hpp"
#include "../../include/coasync/this_coro.hpp"
#include <iostream>

bool ignore_case_equal(char a, char b)
{
  return std::tolower(a) == std::tolower(b);
}

using namespace coasync;
using std::chrono::operator""s;
awaitable<void> connetion(net::tcp::socket client) noexcept
{
  std::puts("hello client");
  co_spawn(co_await this_coro::context,
           [](net::tcp::socket* _client) -> awaitable<void>
  {
  	co_await sleep_for(3s); /// after 3s actively close connection
    _client->cancel();
    co_return;
  }(&client),
  use_detach);
  while(true)
    {
      try
        {
          std::cout << co_await net::receive_until(client, "fuck", ignore_case_equal) << "\n";
        }
      catch(std::exception& error)
        {
          std::cout << error.what() << "\n";
          co_return;
        }

    }
}
awaitable<void> acceptance() noexcept
{
  net::tcp::acceptor acceptor
  {
    co_await this_coro::context,
    net::tcp::endpoint(net::address_v4::loopback(), 10086)
  };
  while(true)
    {
      co_spawn(co_await this_coro::context, connetion(co_await acceptor.accept()), use_detach);
      std::puts("new client found");
    }
}
int main()
{
  execution_context context {concurrency_arg(0)};
  co_spawn(context, acceptance(), use_detach);
  context.loop();
}
