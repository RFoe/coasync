#include "../include/coasync/co_signal.hpp"
#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/functional.hpp"
#include "../include/coasync/this_coro.hpp"

#include <iostream> /// for extern std::clog

using namespace coasync;
using std::chrono::operator""s;

awaitable<void> do_raise(int seconds) noexcept
{
  co_await sleep_for(std::chrono::seconds(seconds));
  std::puts("std::raise");
  std::raise(SIGINT);
}
int main()
{
  execution_context context {0};
  co_signal<SIGINT>(context, []
  ([[maybe_unused]]int) noexcept {
    std::puts("interpreted!!! callback 001");
  });
  co_signal<SIGINT>(context, []
  ([[maybe_unused]]int) noexcept {
    std::puts("interpreted!!! callback 002");
  });
  co_spawn(context, do_raise(1), use_detach);
  co_spawn(context, do_raise(2), use_detach);
  context.loop();
  
  std::puts("final raise");
  std::raise(SIGINT); /// see what will happen
}
