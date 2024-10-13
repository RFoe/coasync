#ifndef COASYNC_WHEN_ANY_INCLUDED
#define COASYNC_WHEN_ANY_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "detail/service/flag_service.hpp"
#include "detail/suspendible.hpp"
#include "detail/object_deduce.hpp"
#include "set_stop_source.hpp"
#include "co_spawn.hpp"
#include "awaitable_group.hpp"
#include <variant>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
template <typename Ref, typename Alloc>
COASYNC_ATTRIBUTE((nodiscard))
awaitable<void> when_any_wrapper(
  awaitable<Ref, Alloc> 						a,
  std::shared_ptr<std::atomic_flag> flag_arrive,
  std::size_t const 								this_place,
  manual_lifetime<detail::object_deduce_t<Ref>>& first_value,
  std::atomic_size_t& 							first_arrive,
  std::exception_ptr& 							first_eptr)
{
  manual_lifetime<Ref> prepare_value;
  std::exception_ptr   prepare_eptr {};
  try
    {
      if constexpr(std::is_void_v<Ref>) co_await a;
      else prepare_value.construct(static_cast<Ref &&>(co_await a));
    }
  catch(...)
    {
      prepare_eptr = std::current_exception();
    }
  if(not flag_arrive -> test_and_set(std::memory_order_release)) COASYNC_ATTRIBUTE((unlikely))
    {
      if constexpr(not std::is_void_v<Ref>)
        first_value.construct(static_cast<Ref &&>(prepare_value.get()));
      if(prepare_eptr) COASYNC_ATTRIBUTE((unlikely)) first_eptr = std::move(prepare_eptr);
      first_arrive.store(this_place, std::memory_order_release);
    }
}
template <typename ... RefTypes, typename... AllocTypes, std::size_t... Indices>
COASYNC_ATTRIBUTE((nodiscard))
awaitable<std::variant< detail::object_deduce_t<RefTypes> ...>>
    when_any_impl(
      COASYNC_ATTRIBUTE((maybe_unused)) std::index_sequence<Indices...>,
      awaitable<RefTypes, AllocTypes>... aargs)
{
  std::shared_ptr<std::atomic_flag> 	flag_arrive = std::make_shared<std::atomic_flag>();
  std::stop_source 										ssource;
  std::exception_ptr 									first_eptr {};
  std::atomic_size_t 									first_arrive {std::size_t(-1)};
  execution_context& 									context = co_await get_context();
  manual_lifetime<std::variant< detail::object_deduce_t<RefTypes> ...>>
      desired_value;
  std::tuple< manual_lifetime<detail::object_deduce_t<RefTypes>> ...>
      candidates;
  flag_arrive -> clear();
  (co_spawn(
     context,
     when_any_wrapper(std::move(aargs), flag_arrive, Indices, std::get<Indices>(candidates), first_arrive, first_eptr)
     | set_stop_source(ssource), use_detach_t()), ...);
  co_await suspendible<flag_service>()(* flag_arrive);
  ssource.request_stop();
  while(first_arrive.load(std::memory_order_acquire) == std::size_t(-1))
    std::this_thread::yield();
  if(first_eptr) COASYNC_ATTRIBUTE((unlikely)) std::rethrow_exception(std::move(first_eptr));
  ((first_arrive == Indices
    and (desired_value.construct(std::in_place_index<Indices>, std::move(std::get<Indices>(candidates).get())), false)), ...);
  co_return std::move(desired_value.get());
}
template <typename Ref, typename Alloc>
COASYNC_ATTRIBUTE((nodiscard))
awaitable<detail::object_deduce_t<Ref>>
                                     COASYNC_API when_any_impl(awaitable_group<Ref, Alloc> agroup)
{
  std::shared_ptr<std::atomic_flag> 	flag_arrive = std::make_shared<std::atomic_flag>();
  std::stop_source 										ssource;
  std::exception_ptr 									first_eptr {};
  std::atomic_size_t 									first_arrive {std::size_t(-1)};
  execution_context& 									context = co_await detail::get_context();
  detail::manual_lifetime<detail::object_deduce_t<Ref>> desired_value;
  flag_arrive -> clear();
  for(awaitable<Ref, Alloc>& a: agroup)
    co_spawn(
      context,
      when_any_wrapper(std::move(a), flag_arrive, 0, desired_value, first_arrive, first_eptr),
      detail::use_detach_t());
  co_await detail::suspendible<detail::flag_service>()(* flag_arrive);
  ssource.request_stop();
  while(first_arrive.load(std::memory_order_acquire) == std::size_t(-1))
    std::this_thread::yield();
  if(first_eptr) COASYNC_ATTRIBUTE((unlikely))
    std::rethrow_exception(std::move(first_eptr));
  co_return std::move(desired_value.get());
}
}
/// Create a awaitable object that becomes ready when at least one of the input awaitables become ready.
/// The behavior is undefined if any awaitable future is invalid.
template <typename... RefTypes, typename... AllocTypes>
COASYNC_ATTRIBUTE((nodiscard))
awaitable<std::variant< detail::object_deduce_t<RefTypes> ...>>
    COASYNC_API when_any(awaitable<RefTypes, AllocTypes>... aargs)
{
  return detail::when_any_impl(std::make_index_sequence<sizeof...(RefTypes)>(), std::move(aargs) ...);
}
template <typename Ref, typename Alloc>
COASYNC_ATTRIBUTE((nodiscard))
awaitable<detail::object_deduce_t<Ref>> COASYNC_API when_any(awaitable_group<Ref, Alloc> agroup)
{
  return detail::when_any_impl(std::move(agroup));
}
}
#endif
