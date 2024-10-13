#include "../include/coasync/functional.hpp"
#include "../include/coasync/this_coro.hpp"
#include "../include/coasync/co_spawn.hpp"

using namespace coasync;
using std::chrono::operator""s;

awaitable<int> predicate_X() {
	co_await sleep_for(1s);
	std::puts("predicate X");
	co_return 999;
}
awaitable<int> predicate_Y() {
	co_await sleep_for(2s);
	std::puts("predicate Y");
	co_return 888;
}
awaitable<void> test()
{
	auto [x, y] = co_await (predicate_X() && predicate_Y());
	std::printf("[%d, %d]\n", x, y);
	auto z = co_await (predicate_X() || predicate_Y());
	std::visit([](int v) { std::printf("%d\n", v); }, z);
}


int main() {
  execution_context context{concurrency_arg(0)};
  /// Initiate no child threads
  co_spawn(context, test(), use_detach);
  context.loop();
}
