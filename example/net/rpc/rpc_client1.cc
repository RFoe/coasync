#include "../../../include/coasync/execution_context.hpp"
#include "../../../include/coasync/this_coro.hpp"
#include "../../../include/coasync/co_spawn.hpp"
#include "../../../include/coasync/net/socket.hpp"
#include "../../../include/coasync/net/protocol.hpp"
#include "../../../include/coasync/net/endpoint.hpp"
#include "../../../include/coasync/net/rpc/rpc_client.hpp"
using namespace coasync;

awaitable<void> test() noexcept
{
  net::tcp::socket socket { co_await this_coro::context, net::tcp::v4() };
  co_await socket.connect(net::tcp::endpoint{net::address_v4::loopback(), 10086});
  std::puts("connected");
  net::rpc::rpc_client s { std::move(socket) };
  std::printf("[888 + 999 = %d]\n", co_await s.call<int>("add", 888, 999));
}

int main()
{
  execution_context ctx {concurrency_arg(0)};
  co_spawn(ctx, test(), use_detach);
  ctx.loop();
}
