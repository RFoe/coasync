#ifndef COASYNC_FRAME_BASE_INCLUDED
#define COASYNC_FRAME_BASE_INCLUDED
#include "config.h"
#if __cpp_impl_coroutine >= 201902 && __cpp_lib_coroutine >= 201902
#  include <coroutine>
#elif defined(__cpp_coroutines) && __has_include(<experimental/coroutine>)
#  include <experimental/coroutine>
namespace std
{
using std::experimental::coroutine_handle;
using std::experimental::coroutine_traits;
using std::experimental::noop_coroutine;
using std::experimental::suspend_always;
using std::experimental::suspend_never;
} // namespace std
#else
# error This library requires the use of C++20 coroutine support
#endif
#if defined(__cpp_lib_semaphore)
# include <semaphore>
#else
# error This library requires the use of C++20 semaphore support
#endif
#if defined(__cpp_lib_jthread)
# include <stop_token>
#else
# error This library requires the use of C++20 jthread/stop_token support
#endif
#include <exception>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
struct execution_context;
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
struct awaitable_frame_base
{
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API set_exception(std::exception_ptr eptr) noexcept
  {
    _M_exception = eptr;
  }
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API set_error(std::error_code const& ec) noexcept
  {
    _M_exception = std::make_exception_ptr(std::system_error(ec));
  }
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API rethrow_exception()
  {
    if(_M_exception) COASYNC_ATTRIBUTE((unlikely))
      std::rethrow_exception(std::move(_M_exception));
  }
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API set_context(execution_context* context) noexcept
  {
    _M_context = context;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  execution_context* COASYNC_API get_context() noexcept
  {
    return _M_context;
  }
  COASYNC_ATTRIBUTE((always_inline))
  void set_id(std::uint_least32_t uint) noexcept
  {
    _M_id = uint;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  std::uint_least32_t COASYNC_API get_id() const noexcept
  {
    return _M_id;
  }
  /// std::binary_semaphore:
  /// 	It is used to maintain a strict serial order in the process
  /// 	of swapping out and swapping in each coroutine group in a multithreaded environment
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API set_semaphore(std::binary_semaphore* semaphore) noexcept
  {
    _M_semaphore = semaphore;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  std::binary_semaphore* COASYNC_API get_semaphore() const noexcept
  {
    return _M_semaphore;
  }
  /// std::stop_token
  /// 	A user-visible cancel handle. It is checkable if cancellation is requested at a particular time
  /// 	as a cancellation indication, and thus explicitly exits this coroutine
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API set_stop_token(std::stop_token stoken) noexcept
  {
    if(stoken.stop_possible()) COASYNC_ATTRIBUTE((unlikely))
      _M_stop_token = std::move(stoken);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  std::stop_token COASYNC_API get_stop_token() const noexcept
  {
    return _M_stop_token;
  }
  /// Sets the parent coroutine and passes down the control information
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API push_frame(std::coroutine_handle<awaitable_frame_base> previous_frame) noexcept
  {
    _M_previous = previous_frame;
    set_id(					previous_frame.promise().get_id()					);
    set_context(		previous_frame.promise().get_context()		);
    set_semaphore(	previous_frame.promise().get_semaphore()	);
    set_stop_token(	previous_frame.promise().get_stop_token()	);
  }
  std::suspend_always initial_suspend() const noexcept
  {
    return std::suspend_always{};
  }
  auto final_suspend() const noexcept
  {
    struct final_result
    {
      bool await_ready() const noexcept
      {
        return false;
      }
      std::coroutine_handle<> await_suspend(COASYNC_ATTRIBUTE((maybe_unused)) std::coroutine_handle<>) const noexcept
      {
        return _M_previous ? _M_previous : std::noop_coroutine();
      }
      void await_resume() const noexcept { }
      std::coroutine_handle<> _M_previous;
    };
    return final_result {_M_previous};
  }
  void unhandled_exception()
  {
    if (not _M_previous) COASYNC_ATTRIBUTE((unlikely))
      std::rethrow_exception(std::current_exception());
    else COASYNC_ATTRIBUTE((likely))
      set_exception(std::current_exception());
  }
protected:
  std::coroutine_handle<awaitable_frame_base>
  _M_previous;
  std::exception_ptr									_M_exception;
  execution_context* 									_M_context;
  std::uint_least32_t									_M_id;
  std::binary_semaphore* 							_M_semaphore;
  std::stop_token											_M_stop_token;
};
}
}
#endif
