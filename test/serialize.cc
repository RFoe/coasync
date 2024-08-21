#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/co_spawn.hpp"
#include "../include/coasync/this_coro.hpp"
#include "../include/coasync/net/socket.hpp"
#include "../include/coasync/net/protocol.hpp"
#include "../include/coasync/net/endpoint.hpp"
#include "../include/coasync/net/serde_stream.hpp"
#include <iostream>
using namespace coasync;
awaitable<void> serial()
{
  net::tcp::socket socket { co_await this_coro::context, net::tcp::v4() };
  co_await socket.connect(net::tcp::endpoint{net::address_v4::loopback(), 10086});
  net::serde_stream s { std::move(socket) };
  std::vector<int> vec {};
  for(unsigned int count {}; count < 10; count ++) vec.push_back(count);
  co_await s.serialize(vec);
	co_await s.flush();
  std::queue<int> que {};
  for(unsigned int count {}; count < 10; count ++) que.push(count);
  co_await s.serialize(que);
	co_await s.flush();
//	while(true) std::getchar();
}
int main()
{
  execution_context context {0};
  co_spawn(context, serial(), use_detach);
  context.loop();
}
