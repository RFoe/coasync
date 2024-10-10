#include "../include/coasync/functional.hpp"
#include "../include/coasync/this_coro.hpp"
#include "../include/coasync/co_spawn.hpp"

using namespace coasync;
using std::chrono::operator""s;
std::atomic_flag flag;
std::promise<void> promise;
std::future<void> future = promise.get_future();
std::latch latch{1};
awaitable<void> waiting()
{
	co_await flag;
	std::puts("atomic_flag is set");
	co_await future;
	std::puts("future is set");
	co_await latch;
	std::puts("latch is set");
}
awaitable<void> calling()
{
	co_await 1s;
	flag.test_and_set(std::memory_order_release);
	co_await 1s;
	promise.set_value();
	co_await 1s;
	latch.count_down();
}

int main() {
  execution_context context{concurrency_arg(0)};
  /// Initiate no child threads
  co_spawn(context, waiting(), use_detach);
  co_spawn(context, calling(), use_detach);
  context.loop();
}
