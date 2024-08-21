#include "../include/coasync/this_coro.hpp"
#include "../include/coasync/co_spawn.hpp"
#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/net/socket.hpp"
#include "../include/coasync/net/protocol.hpp"
#include "../include/coasync/net/endpoint.hpp"
#include "../include/coasync/net/serde_stream.hpp"
using namespace coasync;

awaitable<void> test() noexcept {
	net::tcp::socket socket { co_await this_coro::context, net::tcp::v4() };
  co_await socket.connect(net::tcp::endpoint{net::address_v4::loopback(), 10086});
	net::serde_stream s { std::move(socket) };
	std::string str {"add"};
	co_await s.serialize(str);
	std::tuple<int, int> args{3, 5};
	co_await s.serialize(args);
	int value;
	co_await s.deserialize(value);
	std::printf("[3+5=%d]\n", value);
}

int main() {
	execution_context context {0};
	co_spawn(context, test(), use_detach);
	context.loop();
}
