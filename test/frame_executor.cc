#include "../include/coasync/awaitable.hpp"
#include "../include/coasync/detail/frame_executor.hpp"
using namespace coasync;
using namespace coasync::detail;
using std::chrono::operator""s;
awaitable<void> submission() noexcept
{
  std::puts("submission start");
  std::this_thread::sleep_for(1s);
  std::puts("submission complete");
  co_return;
}
awaitable<void> test(frame_executor& ex) noexcept
{
  std::puts("hello world");
  while(true)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      auto frame = std::coroutine_handle<awaitable_frame_base>::from_address(submission().release_coroutine().address());
      ex.push_frame(frame);
    }
  co_return;
}
int main()
{
  frame_executor ex(8);
  for(unsigned int i {}; i < 3; i ++)
    {
      auto frame = std::coroutine_handle<awaitable_frame_base>::from_address(test(ex).release_coroutine().address());
      ex.push_frame(frame);
    }
  ex.request_launch();
  while(true)
    {
      ex.dispatch_frame();
    }
  std::getchar();
  ex.request_stop();
}
