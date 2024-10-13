#include "../include/coasync/functional.hpp"
#include "../include/coasync/this_coro.hpp"
#include "../include/coasync/co_spawn.hpp"

using namespace coasync;
using std::chrono::operator""s;

awaitable<void> void_delay() {
	co_await sleep_for(2s);
}
awaitable<int> int_delay() {
	co_await sleep_for(2s);
	co_return 888;
}
awaitable<void> test()
{
	auto a = void_delay() | [] { std::puts("void_functor"); };
	auto b = int_delay() | [](int v) { return v + 999; };
	co_await a;
	std::printf("%d\n", co_await b);
}


int main() {
  execution_context context{concurrency_arg(0)};
  /// Initiate no child threads
  co_spawn(context, test(), use_detach);
  context.loop();
}
