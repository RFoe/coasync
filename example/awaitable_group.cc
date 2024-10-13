#include "../include/coasync/when_all.hpp"
#include "../include/coasync/when_any.hpp"
#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/functional.hpp"
#include "../include/coasync/this_coro.hpp"

using namespace coasync;
using std::chrono::operator""s;

awaitable<int> delay(int seconds)
{
  co_await sleep_for(std::chrono::seconds(seconds));
  std::puts("sleep awaiken");
  co_return co_await this_coro::id;
}
awaitable<void> test()
{
  for(unsigned int i {}; i < 10; i ++)
    {
  awaitable_group<int> group;
  for(unsigned int i {}; i < 5; i ++)
    group.emplace_back(delay(i));
  auto results = co_await when_all(std::move(group));
	std::printf("when_all return, size: %llu\n", results.size());
  for(int res: results)
    std::printf("res: %d\n", res);
    }
  co_return;
}
int main()
{
  execution_context context{concurrency_arg(3)};
  /// Initiate three child threads
  co_spawn(context, test(), use_detach);
  context.loop();
}
