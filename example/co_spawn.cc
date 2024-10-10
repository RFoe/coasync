#include "../include/coasync/co_spawn.hpp"
#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/functional.hpp"
#include "../include/coasync/this_coro.hpp"
using namespace coasync;
using std::chrono::operator""s;
awaitable<void> test() noexcept
{
  std::puts("test");
  auto s = co_await this_coro::stop_token;
  auto a = co_spawn(co_await this_coro::context, sleep_for(2s), use_awaitable);
	co_await a;
  std::puts("hello world");
  co_return;
}
int main()
{
  execution_context context {concurrency_arg(0)};
  co_spawn(context, test(), use_detach);
  std::puts("co_spawn");
  context.loop();
  return 0;
}
