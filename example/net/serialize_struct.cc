#include "../../include/coasync/execution_context.hpp"
#include "../../include/coasync/co_spawn.hpp"
#include "../../include/coasync/this_coro.hpp"
#include "../../include/coasync/net/socket.hpp"
#include "../../include/coasync/net/protocol.hpp"
#include "../../include/coasync/net/endpoint.hpp"
#include "../../include/coasync/net/serde_stream.hpp"
#include <iostream>
using namespace coasync;

struct example
{
  int ivalue;
  double fvalue;
  std::vector<int> ivector;
};
struct_meta(example, ivalue, fvalue, ivector);

awaitable<void> serial()
{
  net::tcp::socket socket { co_await this_coro::context, net::tcp::v4() };
  co_await socket.connect(net::tcp::endpoint{net::address_v4::loopback(), 10086});
  net::serde_stream s { std::move(socket) };
  example ex {.ivalue = 888, .fvalue = 3.14, .ivector = {1, 2, 3}};
  co_await s.serialize(ex);
}

int main()
{
  execution_context context {concurrency_arg(2)};
  co_spawn(context, serial(), use_detach);
  context.loop();
}
