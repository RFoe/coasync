#include "../include/coasync/co_spawn.hpp"
#include "../include/coasync/co_mutex.hpp"
#include "../include/coasync/co_condition_variable.hpp"
#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/functional.hpp"
#include "../include/coasync/this_coro.hpp"
#include <queue>
#include <iostream>

using namespace coasync;
using std::chrono::operator""s;

co_mutex mutex;
co_condition_variable condition;
std::queue<int> data; /// unprotected raw data

awaitable<void> producer()
{
  co_await mutex.lock();
  std::puts("producer");
  data.emplace(co_await this_coro::id);
  co_await sleep_for(1s);
  mutex.unlock();
  co_await condition.notify_one();
  co_return;
}

awaitable<void> consumer()
{
  co_await mutex.lock();
  co_await condition.wait(mutex, [&] { return not data.empty(); });
  assert(not data.empty());
  std::printf("consumer: %d\n", data.front());
  data.pop();
  mutex.unlock();
  co_return;
}

int main()
{
  execution_context context {8}; /// data race
  for(unsigned int i {}; i < 16; i ++)
    co_spawn(context, producer(), use_detach);
  for(unsigned int i {}; i < 16; i ++)
    co_spawn(context, consumer(), use_detach);

  context.loop();
  return 0;
}
