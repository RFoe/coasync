#include "../include/coasync/when_all.hpp"
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
  for(unsigned int i {}; i < 50; i ++)
    {
      auto [a, b, c] = co_await when_all(delay(1), delay(2), delay(3));
      std::printf("[%d, %d, %d]\n", a, b, c);
    }
  co_return;
}
int main()
{
  execution_context context{};
  co_spawn(context, test(), use_detach);
  context.loop();
}
