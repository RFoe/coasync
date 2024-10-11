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
/// the set of messages supported by a basic_channel is specified by its template parameters.
/// Messages can be sent and received using asynchronous or non-blocking synchronous operations.
template <typename execution_context, typename Value, std::size_t Bound, typename Mutex = detail::basic_lockable>
struct COASYNC_ATTRIBUTE((nodiscard)) basic_channel
{
  /// An async multi-producer multi-consumer basic_channel, where each message can be received
  /// by only one of all existing consumers.
  /// Bounded basic_channel with limited capacity.
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
  typedef typename ring_container<Value, Bound>::reference reference;
  typedef typename ring_container<Value, Bound>::const_reference const_reference;
  typedef typename ring_container<Value, Bound>::size_type size_type;

  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit basic_channel(execution_context& context) noexcept
    : _M_context(context) {}
  constexpr basic_channel& operator=(basic_channel const&) = delete;
  constexpr basic_channel(basic_channel const&) = delete;
  COASYNC_ATTRIBUTE((always_inline)) basic_channel(basic_channel&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) basic_channel& operator=(basic_channel&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) ~ basic_channel() noexcept = default;

  template <typename... CtorArgs>
  requires (not(sizeof...(CtorArgs) == 1 and (std::is_same_v<CtorArgs, Value> || ...)))
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<void> COASYNC_API send(CtorArgs&& ... args)
  {
    static_assert(std::constructible_from<Value, CtorArgs ...>);
    Value local_value {  std::forward<CtorArgs&&>(args) ...};
    co_await detail::related_suspendible<detail::enqueue_service>(_M_context)(_M_queue, local_value, _M_mutex, Bound);
  }
  template <typename OtherValue> requires std::is_same_v<std::remove_cvref_t<OtherValue>, Value>
  COASYNC_ATTRIBUTE((nodiscard)) awaitable<void> COASYNC_API send(OtherValue&& value)
  {
    co_await detail::related_suspendible<detail::enqueue_service>(_M_context)(_M_queue, const_cast<Value&>(value), _M_mutex, Bound);
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
    co_await detail::related_suspendible<detail::dequeue_service>(_M_context)(_M_queue, storage._M_value, _M_mutex);
    co_return std::move(storage._M_value);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) static constexpr std::size_t capacity() noexcept
  {
    return Bound;
  }

  COASYNC_ATTRIBUTE((nodiscard, always_inline)) execution_context& context() noexcept
  {
    return _M_context;
  }
private:
  COASYNC_ATTRIBUTE((no_unique_address)) execution_context& _M_context;
  mutable Mutex 						_M_mutex;
  std::queue<Value, ring_container<Value, Bound>> 				_M_queue;
};
template <typename Value, std::size_t Bound, typename Mutex>
using channel
  = basic_channel<execution_context, Value, Bound, Mutex>;
}
#endif
