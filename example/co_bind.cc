#include "../include/coasync/co_bind.hpp"
#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/when_any.hpp"
#include "../include/coasync/when_all.hpp"
#include "../include/coasync/functional.hpp"
#include "../include/coasync/this_coro.hpp"
using namespace coasync;
using std::chrono::operator""s;
awaitable<void> co_sleep(int seconds) noexcept
{
  co_await sleep_for(std::chrono::seconds(seconds));
  std::puts("co_await sleep_for");
}
void sleep(int seconds) noexcept
{
  std::this_thread::sleep_for(std::chrono::seconds(seconds));
  std::puts("this_thread::sleep_for");
}
awaitable<void> test() noexcept {
 execution_context& ctx = co_await this_coro::context;
	for(unsigned int i {}; i < 10; i ++) {
		co_await when_any(co_sleep(1), co_sleep(3), co_bind(ctx, &sleep, 2), co_bind(ctx, sleep, 4));
		std::puts("----------when_any done----------");
		co_await when_all(co_sleep(1), co_sleep(3), co_bind(ctx, sleep, 2), co_bind(ctx, sleep, 4));
		std::puts("----------when_all done----------");
	}
}
int main()
{
 execution_context context;
 co_spawn(context, test(), use_detach);
 context.loop();
}
