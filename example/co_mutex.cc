#include "../include/coasync/co_spawn.hpp"
#include "../include/coasync/co_mutex.hpp"
#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/functional.hpp"
#include "../include/coasync/this_coro.hpp"
#include <iostream>

using namespace coasync;
using std::chrono::operator""s;


int data; /// unprotected raw data

awaitable<void> access(co_mutex& mutex) noexcept
{
  std::puts("access");
  auto _l = co_await mutex.scoped();
//  co_await mutex.lock();
  std::printf("data: %d\n", data ++);
  std::cout << "thread: " << std::this_thread::get_id() << "\n";
  co_await sleep_for(1s);
//  mutex.unlock();
  co_return;
}
int main()
{
  execution_context context {concurrency_arg(8)}; /// data race
  co_mutex mutex{context};
  for(unsigned int i {}; i < 16; i ++)
    co_spawn(context, access(mutex), use_detach);
  context.loop();
  return 0;
}
