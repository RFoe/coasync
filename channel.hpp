#ifndef COASYNC_CHANNEL_INCLUDED
#define COASYNC_CHANNEL_INCLUDED
#include "detail/suspendible.hpp"
#include "detail/service/dequeue_service.hpp"
#include "detail/service/enqueue_service.hpp"
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
/// may be used to send messages between different parts of the same application.
/// the set of messages supported by a channel is specified by its template parameters.
/// Messages can be sent and received using asynchronous or non-blocking synchronous operations.
template <typename Value, std::size_t Bound, typename Mutex = detail::basic_lockable>
struct channel
{
	/// An async multi-producer multi-consumer channel, where each message can be received
	/// by only one of all existing consumers.
	/// Bounded channel with limited capacity.
  static_assert(
    std::is_default_constructible_v<Mutex>
    and requires(Mutex& mutex)
  {
    mutex.lock();
    mutex.unlock();
  });
  template <typename... CtorArgs> requires std::constructible_from<Value, CtorArgs ...>
  COASYNC_ATTRIBUTE((nodiscard))
	awaitable<void> COASYNC_API send(CtorArgs&& ... args)
  {
    Value local_value {  std::forward<CtorArgs&&>(args) ...};
    co_await detail::suspendible<detail::enqueue_service>()(_M_queue, local_value, _M_mutex, Bound);
  }
  COASYNC_ATTRIBUTE((nodiscard))
	awaitable<Value> COASYNC_API receive()
  {
  	/// see bug report: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=103790
  	/// Bug 103790 - internal compiler error: Segmentation fault when playing with coroutine
  	/// fixed until GCC 11.1.0
  	/// declaration of anonymous union in coroutine causes internal compiler error segmentation fault
    COASYNC_ATTRIBUTE((gnu::uninitialized)) union Storage
    {
      Value _M_value;
    } storage;
    co_await detail::suspendible<detail::dequeue_service>()(_M_queue, storage._M_value, _M_mutex);
    co_return std::move(storage._M_value);
  }
private:
  mutable Mutex 						_M_mutex;
  std::queue<Value> 				_M_queue;
};
}
#endif
