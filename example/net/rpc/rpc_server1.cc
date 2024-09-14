#include "../../../include/coasync/execution_context.hpp"
#include "../../../include/coasync/this_coro.hpp"
#include "../../../include/coasync/net/endpoint.hpp"
#include "../../../include/coasync/net/protocol.hpp"
#include "../../../include/coasync/net/acceptor.hpp"
#include "../../../include/coasync/net/rpc/rpc_server.hpp"
using namespace coasync;
awaitable<void> test() noexcept {
  net::tcp::acceptor acceptor
  {
    co_await this_coro::context,
    net::tcp::endpoint(net::address_v4::loopback(), 10086)
  };
  net::rpc::rpc_server server(std::move(acceptor));
  server.bind("add", [](int a, int b) -> int { return a + b; });
  co_await server.start();
}
int main() {
 	execution_context ctx;
	co_spawn(ctx, test(), use_detach);
	ctx.loop();
}
