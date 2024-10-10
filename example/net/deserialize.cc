#include "../../include/coasync/execution_context.hpp"
#include "../../include/coasync/co_spawn.hpp"
#include "../../include/coasync/this_coro.hpp"
#include "../../include/coasync/net/endpoint.hpp"
#include "../../include/coasync/net/acceptor.hpp"
#include "../../include/coasync/net/protocol.hpp"
#include "../../include/coasync/net/serde_stream.hpp"

using namespace coasync;

awaitable<void> deserial(net::tcp::socket socket) {
	net::serde_stream s { std::move(socket) };
	std::vector<int> vec;
	std::queue<int>  que;
	co_await s.deserialize(vec);
	for(auto value: vec)
		std::printf("vector %d\n", value);
	co_await s.deserialize(que);
	while(not que.empty()) {
		printf("queue: %d\n", que.front());
		que.pop();
	}
}

awaitable<void> acceptance() noexcept
{
  net::tcp::acceptor acceptor {
    co_await this_coro::context,
    net::tcp::endpoint(net::address_v4::loopback(), 10086)
  };
  while(true)
    co_spawn(co_await this_coro::context, deserial(co_await acceptor.accept()), use_detach);
}

int main() {
	  execution_context context {concurrency_arg(2)};
	  co_spawn(context, acceptance(), use_detach);
	  context.loop();
}
