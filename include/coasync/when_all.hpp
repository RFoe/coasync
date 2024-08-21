#ifndef COASYNC_WHEN_ALL_INCLUDED
#define COASYNC_WHEN_ALL_INCLUDED
#include "detail/service/latch_service.hpp"
#include "detail/suspendible.hpp"
#include "detail/object_deduce.hpp"
#include "co_spawn.hpp"
#include <vector>
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
template <typename Ref, typename Alloc>
COASYNC_ATTRIBUTE((nodiscard))
awaitable<void> when_all_wrapper(
  awaitable<Ref, Alloc> a,
  std::latch& local_latch,
  manual_lifetime<object_deduce_t<Ref>>& desired_value,
  std::vector<std::exception_ptr>& eptr_collection,
  basic_lockable& eptr_lockable)
{
  try
    {
      if constexpr(std::is_void_v<Ref>) co_await a;
      else desired_value.construct(static_cast<Ref&&>(co_await a));
    }
  catch(...)
    {
      std::lock_guard l { eptr_lockable };
      eptr_collection.emplace_back(std::current_exception());
    }
  local_latch.count_down();
}
template <typename... RefTypes, typename... AllocTypes, std::size_t... Indices>
COASYNC_ATTRIBUTE((nodiscard))
awaitable<std::tuple<object_deduce_t<RefTypes> ...>>
    when_all_impl(COASYNC_ATTRIBUTE((maybe_unused)) std::index_sequence<Indices ...>, awaitable<RefTypes, AllocTypes>... aargs)
{
  std::latch 											local_latch { sizeof...(Indices) };
  std::vector<std::exception_ptr> eptr_collection;
  basic_lockable 									eptr_lockable;
  execution_context&       				context = co_await get_context();
  std::tuple<manual_lifetime<object_deduce_t<RefTypes>> ...> values;
  eptr_collection.reserve(sizeof...(Indices));
  (co_spawn(
     context,
     when_all_wrapper(std::move(aargs), local_latch, std::get<Indices>(values), eptr_collection, eptr_lockable), use_detach_t()), ...);
  co_await suspendible<latch_service>()(local_latch);
  if(not eptr_collection.empty()) COASYNC_ATTRIBUTE((unlikely))
    std::rethrow_exception(std::make_exception_ptr(std::move(eptr_collection)));
  co_return std::make_tuple(std::move(std::get<Indices>(values).get()) ...);
}
}
/// Create a awaitable object that becomes ready when all of the input awaitables become ready.
/// The behavior is undefined if any input awaitable is invalid.
template <typename... RefTypes, typename... AllocTypes>
COASYNC_ATTRIBUTE((nodiscard))
awaitable<std::tuple<detail::object_deduce_t<RefTypes> ...>>
    COASYNC_API when_all(awaitable<RefTypes, AllocTypes> ...aargs)
{
  return detail::when_all_impl(std::make_index_sequence<sizeof...(RefTypes)>(), std::move(aargs) ...);
}
}
#endif
