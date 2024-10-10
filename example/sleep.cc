#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/functional.hpp"
#include "../include/coasync/this_coro.hpp"
#include "../include/coasync/co_spawn.hpp"

using namespace coasync;
using std::chrono::operator""s;

awaitable<void> test(int seconds)
{
  co_await sleep_for(std::chrono::seconds(seconds));
  std::puts("sleep awaiken");
}

int main() {
  execution_context context{concurrency_arg(0)};
  /// Initiate no child threads
  co_spawn(context, test(1), use_detach);
  co_spawn(context, test(2), use_detach);
  co_spawn(context, test(3), use_detach);
  context.loop();
}
