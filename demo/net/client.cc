#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/co_spawn.hpp"
#include "../include/coasync/net/socket.hpp"
#include "../include/coasync/net/protocol.hpp"
#include "../include/coasync/net/endpoint.hpp"
#include "../include/coasync/this_coro.hpp"
#include <iostream>
using namespace coasync;

awaitable<void> do_send() {
	net::tcp::socket socket { co_await this_coro::context, net::tcp::v4() };
  co_await socket.connect(net::tcp::endpoint{net::address_v4::loopback(), 10086});
	std::string send_bytes;
	while(true) {
		std::getline(std::cin, send_bytes);
		if(send_bytes.compare("quit") == 0) co_return;
		co_await socket.send(send_bytes);
	}
}

int main() {
	execution_context context {0};
	co_spawn(context, do_send(), use_detach);
	context.loop();
}
