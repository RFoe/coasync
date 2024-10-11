#ifndef COASYNC_SELECT_SOCKET_SERVICE_INCLUDED
#define COASYNC_SELECT_SOCKET_SERVICE_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../awaitable_frame_base.hpp"
#include "../basic_lockable.hpp"
#include "../networking.hpp"
#include "../config.hpp"
#include "../../cancellation_error.hpp"
#include <forward_list>
#if __has_include(<ranges>) && \
  (defined(__cpp_lib_ranges) && __cpp_lib_ranges >= 201911L) && \
  (!COASYNC_CLANG() || __clang_major__ >= 16 || defined(_LIBCPP_VERSION))
#  include <ranges>
#else
#  error This library requires the use of C++20 ranges support
#endif
#include <mutex>
#include <map>
#if defined(__has_include)
# if defined(__linux__)
#  if __has_include(<sys/select.h>)
#   include <sys/select.h>
#  endif
# endif
#endif

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
template <typename execution_context, bool subscribe_sockin>
struct basic_select_socket_service
{
  typedef basic_lockable mutex_type;

  COASYNC_ATTRIBUTE((always_inline))
  constexpr explicit basic_select_socket_service(execution_context& context) noexcept
    : _M_context(context)
  {
    FD_ZERO(&_M_fd_set);
  }
  constexpr basic_select_socket_service& operator=(basic_select_socket_service const&) = delete;
  constexpr basic_select_socket_service(basic_select_socket_service const&) = delete;
  COASYNC_ATTRIBUTE((always_inline)) basic_select_socket_service(basic_select_socket_service&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) basic_select_socket_service& operator=(basic_select_socket_service&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) ~ basic_select_socket_service() noexcept = default;

  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  static constexpr std::size_t overlap_arity() noexcept
  {
    return 1u;
  }

  COASYNC_ATTRIBUTE((always_inline)) void post_frame(std::coroutine_handle<awaitable_frame_base> frame, __native_handle sockfd)
  {
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    if(not FD_ISSET(sockfd, &_M_fd_set)) COASYNC_ATTRIBUTE((likely)) FD_SET(sockfd, &_M_fd_set);
    _M_multimap.emplace(sockfd, frame);
  }
  COASYNC_ATTRIBUTE((always_inline)) void commit_frame()
  {
    struct recipient_t
    {
      int 							_M_max_fdsock;
      ::fd_set 					_M_fdset_image;
      int 							_M_select_result;
      ::timeval const 	_M_timevalue { .tv_sec = 0, .tv_usec = 0 };
    } recipient;
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    if(_M_multimap.empty()) COASYNC_ATTRIBUTE((likely)) return;
    else COASYNC_ATTRIBUTE((unlikely)) recipient._M_max_fdsock = _M_multimap.crbegin() -> first + 1;
    recipient._M_fdset_image = _M_fd_set;
    if(alternative_lock.owns_lock()) COASYNC_ATTRIBUTE((likely)) alternative_lock.unlock();
    if constexpr(subscribe_sockin)
      recipient._M_select_result
        = ::select(recipient._M_max_fdsock, &recipient._M_fdset_image, nullptr, nullptr, const_cast<timeval*>(&recipient._M_timevalue));
    else
      recipient._M_select_result
        = ::select(recipient._M_max_fdsock, nullptr, &recipient._M_fdset_image, nullptr, const_cast<timeval*>(&recipient._M_timevalue));
    if(recipient._M_select_result == -1) COASYNC_ATTRIBUTE((unlikely))
      {
        if(detail::get_errno() == get_error_code(EINTR)) COASYNC_ATTRIBUTE((likely)) return;
        throw std::system_error(detail::get_errno(), detail::generic_category());
      }
    std::forward_list<std::coroutine_handle<awaitable_frame_base>> outstanding;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    for(__native_handle sockfd: std::ranges::views::keys(_M_multimap))
      if(FD_ISSET(sockfd, &recipient._M_fdset_image)) COASYNC_ATTRIBUTE((unlikely))
        {
          auto first_it = _M_multimap.equal_range(sockfd).first;
          outstanding.emplace_front(first_it -> second);
          _M_multimap.erase(first_it);
        }
    if(alternative_lock.owns_lock()) COASYNC_ATTRIBUTE((likely)) alternative_lock.unlock();
    for(std::coroutine_handle<awaitable_frame_base> frame: outstanding)
      _M_context.push_frame_to_executor(frame);
  }
  COASYNC_ATTRIBUTE((always_inline)) void COASYNC_API cancel_frame(__native_handle sockfd)
  {
    std::forward_list<std::coroutine_handle<awaitable_frame_base>> deprecated;
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable>              alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    auto [first, last] = _M_multimap.equal_range(sockfd);
    for(std::coroutine_handle<awaitable_frame_base> frame: std::ranges::subrange { first, last }| std::ranges::views::values)
      deprecated.emplace_front(frame);
    _M_multimap.erase(sockfd);
    FD_CLR(sockfd, &_M_fd_set);
    if(alternative_lock.owns_lock()) COASYNC_ATTRIBUTE((likely)) alternative_lock.unlock();
    if(deprecated.empty()) COASYNC_ATTRIBUTE((unlikely))
      throw cancellation_error(cancellation_errc::no_frame_registered);
    for(std::coroutine_handle<awaitable_frame_base> frame: deprecated)
      {
        std::exception_ptr eptr = std::make_exception_ptr(cancellation_error(cancellation_errc::cancellation_requested));
        frame.promise().set_exception(std::move(eptr));
        _M_context.push_frame_to_executor(frame);
      }
  }
private:
  execution_context& 			_M_context;
  mutable basic_lockable 	_M_lockable;
  std::multimap<__native_handle, std::coroutine_handle<awaitable_frame_base>>
      _M_multimap;
  ::fd_set 								_M_fd_set;
};
typedef basic_select_socket_service<execution_context, true> 		select_socketin_service;
typedef basic_select_socket_service<execution_context, false> 	select_socketout_service;
}
}
#endif
