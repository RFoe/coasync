#ifndef COASYNC_SET_STOP_SOURCE_INCLUDED
#define COASYNC_SET_STOP_SOURCE_INCLUDED
#include "awaitable.hpp"
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
/// intended to associate a std::stop_source with an awaitable object using pipeline operator|.
/// It enables the coroutine to listen for stop requests and propagate them through the
/// awaitable chain.
struct set_stop_source
{
  COASYNC_ATTRIBUTE((always_inline))
  explicit set_stop_source(std::stop_source& source) noexcept
    : _M_stop_source(static_cast<std::stop_source&&>(source))
  {}
  constexpr set_stop_source& operator=(set_stop_source const&) = delete;
  constexpr set_stop_source& operator=(set_stop_source&&) = delete;
  template <typename Ref, typename Alloc>
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend awaitable<Ref, Alloc> COASYNC_API operator|(awaitable<Ref, Alloc> a, set_stop_source&& source)
  {
    a.get_coroutine().promise().set_stop_token(source._M_stop_source.get_token());
    return a;
  }
private:
  std::stop_source&& _M_stop_source;
};
}
#endif
