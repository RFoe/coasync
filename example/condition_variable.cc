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

std::queue<int> data; /// unprotected raw data

awaitable<void> producer(co_mutex& mutex, co_condition_variable& condition)
{
  co_await mutex.lock();
  std::puts("producer");
  data.emplace(co_await this_coro::id);
  co_await sleep_for(1s);
  mutex.unlock();
  condition.notify_one();
  co_return;
}

awaitable<void> consumer(co_mutex& mutex, co_condition_variable& condition)
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
  execution_context context {concurrency_arg(8)}; /// data race
  co_condition_variable condition {context};
  co_mutex mutex{context};
  for(unsigned int i {}; i < 16; i ++)
    co_spawn(context, producer(mutex, condition), use_detach);
  for(unsigned int i {}; i < 16; i ++)
    co_spawn(context, consumer(mutex, condition), use_detach);

  context.loop();
  return 0;
}
