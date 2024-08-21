#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/net/protocol.hpp"
#include "../include/coasync/net/resolver.hpp"
#include "../include/coasync/net/endpoint.hpp"
#include "../include/coasync/this_coro.hpp"
#include <iostream>
using namespace coasync;

awaitable<void> test() noexcept {
	net::tcp::resolver resolver { co_await this_coro::context };
	auto results = co_await resolver.resolve(net::tcp::endpoint(net::address_v4::loopback(), 10086), use_awaitable);
	for(auto& entry: results) {
		std::cout << entry.host_name() << "\n";
		std::cout << entry.service_name() << "\n";
		std::cout << entry.endpoint() << "\n";
	}
}

int main() {
 execution_context context;
 co_spawn(context, test(), use_detach);
 context.loop();
	
	return 0;
}
