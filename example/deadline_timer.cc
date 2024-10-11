#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/deadline_timer.hpp"
#include "../include/coasync/functional.hpp"
#include "../include/coasync/this_coro.hpp"
#include "../include/coasync/co_spawn.hpp"

using namespace coasync;
using std::chrono::operator""s;

awaitable<void> expire(deadline_timer<>& timer)
{
 co_await timer.wait();
 std::puts("wait done or cancel");
}
awaitable<void> cancel(deadline_timer<>& timer) {
	co_await sleep_for(3s);
	timer.cancel();
	std::puts("cancel");
}

int main() {
  execution_context context{concurrency_arg(0)};
  /// Initiate no child threads
  deadline_timer timer(context, 2s);
  co_spawn(context, expire(timer), use_detach);
  co_spawn(context, cancel(timer), use_detach);
  context.loop();
}
