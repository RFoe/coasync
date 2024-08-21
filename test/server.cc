#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/co_spawn.hpp"
#include "../include/coasync/net/endpoint.hpp"
#include "../include/coasync/net/acceptor.hpp"
#include "../include/coasync/net/protocol.hpp"
#include "../include/coasync/functional.hpp"
#include "../include/coasync/this_coro.hpp"

using namespace coasync;

awaitable<void> do_handle(net::tcp::socket client) noexcept
{
  std::puts("hello client");
  std::array<char, 64> 	buffer;
  while(true) {
    int length;
    try {
      length = co_await client.receive(buffer);
    } catch(...) {
      std::puts("trap");
      co_return;
    }
    if(__builtin_expect(length == 0, false)) {
      std::puts("cancel");
      client.close();
      break;
    }
    buffer[length] = '\0';
    std::printf("receive %s\n", std::addressof(buffer[0]));
  }
}

awaitable<void> do_listen() noexcept
{
  net::tcp::acceptor acceptor {
    co_await this_coro::context,
    net::tcp::endpoint(net::address_v4::loopback(), 10086)
  };
  std::printf("ac fd: %zu\n", acceptor.native_handle());

  while(true) {
  	try {
    auto socket = co_await acceptor.accept();
    co_spawn(co_await this_coro::context,
             do_handle(std::move(socket)),
             use_detach);
		} catch(...) {
			std::puts("accept error");
		}
  }
}


int main()
{
  execution_context context {0};
  co_spawn(context, do_listen(), use_detach);
  context.loop();
}
