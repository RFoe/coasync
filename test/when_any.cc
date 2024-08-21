#include "../include/coasync/when_any.hpp"
#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/functional.hpp"
#include "../include/coasync/this_coro.hpp"
using namespace coasync;
using std::chrono::operator""s;
awaitable<int>  delay(int seconds)
{
  std::puts("sleep");
  co_await sleep_for(std::chrono::seconds(seconds));
  std::puts("hello");
  co_return co_await this_coro::id;
}
awaitable<void> test()
{
//  std::puts("test()");
//	co_await sleep_for(1s);
//	std::puts("test()");
//	co_await sleep_for(1s);
//	std::puts("test()");
//	co_await sleep_for(1s);
//	std::puts("test()");
//	co_await delay(1);
//	co_await delay(1);
//	co_await delay(1);
  for(unsigned int i {}; i < 50; i ++)
    {
      auto result = co_await when_any(delay(1), delay(2), delay(3));
      std::printf("index: %llu\n", result.index());
      std::visit([](int value)
      {
        std::printf("result: %d\n", value);
      }, result);
    }
  co_return;
}
int main()
{
  execution_context context{};
  co_spawn(context, test(), use_detach);
  context.loop();
}
