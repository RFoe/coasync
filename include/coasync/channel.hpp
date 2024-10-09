#ifndef COASYNC_CHANNEL_INCLUDED
#define COASYNC_CHANNEL_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "detail/suspendible.hpp"
#include "detail/ring_container.hpp"
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
private:
  static_assert(
    std::is_default_constructible_v<Mutex>
    and requires(Mutex& mutex)
  {
    mutex.lock();
    mutex.unlock();
  });
public:
	typedef Mutex mutex_type;
	typedef ring_container<Value, Bound> container_type;
	typedef typename ring_container<Value, Bound>::value_type value_type;
	typedef typename ring_container<Value, Bound>::reference_type reference_type;
	typedef typename ring_container<Value, Bound>::const_reference_type const_reference_type;
	typedef typename ring_container<Value, Bound>::size_type size_type;
	
  template <typename... CtorArgs>
  requires (not(sizeof...(CtorArgs) == 1 and (std::is_same_v<CtorArgs, Value> || ...)))
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<void> COASYNC_API send(CtorArgs&& ... args)
  {
    static_assert(std::constructible_from<Value, CtorArgs ...>);
    Value local_value {  std::forward<CtorArgs&&>(args) ...};
    co_await detail::suspendible<detail::enqueue_service>()(_M_queue, local_value, _M_mutex, Bound);
  }
  template <typename OtherValue> requires std::is_same_v<std::remove_cvref_t<OtherValue>, Value>
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<void> COASYNC_API send(OtherValue&& value)
  {
    co_await detail::suspendible<detail::enqueue_service>()(_M_queue, const_cast<Value&>(value), _M_mutex, Bound);
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
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) static constexpr std::size_t capacity() noexcept
  {
    return Bound;
  }
private:
  mutable Mutex 						_M_mutex;
  std::queue<Value, ring_container<Value, Bound>> 				_M_queue;
};
}
#endif
