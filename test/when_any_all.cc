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
      auto [a, b, c] = co_await when_all(delay(1), delay(2), delay(3));
      std::printf("when_all results: [%d, %d, %d]\n", a, b, c);
      auto result = co_await when_any(delay(1), delay(2), delay(3));
      std::printf("when_any: index: %llu\n", result.index());
      std::visit([](int value)
      {
        std::printf("when_any: result: %d\n", value);
      }, result);
    }
  co_return;
}
int main()
{
  execution_context context{3};
  /// Initiate three child threads
  co_spawn(context, test(), use_detach);
  context.loop();
}
