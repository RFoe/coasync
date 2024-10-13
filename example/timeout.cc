#include "../include/coasync/functional.hpp"
#include "../include/coasync/this_coro.hpp"
#include "../include/coasync/co_spawn.hpp"

using namespace coasync;
using std::chrono::operator""s;

awaitable<int> task_cost_1s() {
	co_await sleep_for(1s);
	std::puts("task done");
	co_return 999;
}
awaitable<int> task_cost_3s() {
	co_await sleep_for(3s);
		std::puts("task done");
		co_return 888;
}
awaitable<void> test()
{
	std::puts("-------------- task 1 -------------");
	if(auto result = co_await timeout(task_cost_1s(), 10s))
		std::printf("not timeout, value: %d", *result);
	else std::puts("timeout");
	
	
	std::puts("-------------- task 2 -------------");
	if(auto result = co_await timeout(task_cost_3s(), 2s))
		std::printf("not timeout, value: %d", *result);
	else std::puts("timeout");
}


int main() {
  execution_context context{concurrency_arg(0)};
  /// Initiate no child threads
  co_spawn(context, test(), use_detach);
  context.loop();
}
