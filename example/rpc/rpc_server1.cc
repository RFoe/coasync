#include "../../include/coasync/execution_context.hpp"
#include "../../include/coasync/this_coro.hpp"
#include "../../include/coasync/net/endpoint.hpp"
#include "../../include/coasync/net/protocol.hpp"
#include "../../include/coasync/net/acceptor.hpp"
#include "../../include/coasync/rpc/rpc_server.hpp"
using namespace coasync;
using std::chrono::operator""s;
awaitable<void> test() noexcept {
  net::tcp::acceptor acceptor
  {
    co_await this_coro::context,
    net::tcp::endpoint(net::address_v4::loopback(), 10086)
  };
  net::rpc::rpc_server server(std::move(acceptor));
  server.bind("add", [](int a, int b) -> int {
		std::this_thread::sleep_for(2s);
		return a + b;
	});
  co_await server.start();
}
int main() {
 	execution_context ctx {concurrency_arg(0)};
	co_spawn(ctx, test(), use_detach);
	ctx.loop();
}
