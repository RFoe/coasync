#ifndef COASYNC_EPOLL_SOCKET_SERVICE_INCLUDED
#define COASYNC_EPOLL_SOCKET_SERVICE_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if defined(__linux__) && __has_include(<sys/epoll.h>)
#include <sys/epoll.h>

#include "../awaitable_frame_base.hpp"
#include "../basic_lockable.hpp"
#include "../networking.hpp"
#include "../config.hpp"

#include <mutex>
#include <unordered_map>
#include <forward_list>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{

template <typename execution_context, bool subscribe_sockin>
struct basic_epoll_socket_service
{
  COASYNC_ATTRIBUTE((always_inline)) explicit basic_epoll_socket_service(execution_context& context) noexcept: _M_context(context)
  {
    if((_M_epollfd = ::epoll_create1(0)) < 0)
      throw std::system_error(detail::get_errno(), detail::generic_category());
  }
  COASYNC_ATTRIBUTE((always_inline)) void post_frame(std::coroutine_handle<awaitable_frame_base> frame, int sockfd)
  {
    if((_M_fds_bitmap[sockfd / 8] & (1UL << (sockfd % 8))) == 0) COASYNC_ATTRIBUTE((unlikely))
      return;
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    struct ::epoll_event event;
    event.events = subscribe_sockin ? EPOLLIN | EPOLLRDHUP : EPOLLOUT | EPOLLHUP;
    event.data.ptr = static_cast<void*>(frame.address());
    int res = ::epoll_ctl(_M_epollfd, EPOLL_CTL_ADD, sockfd, std::addressof(event));
    if(res == -1)	COASYNC_ATTRIBUTE((unlikely))
      throw std::system_error(detail::get_errno(), detail::generic_category());
    _M_fds_bitmap[sockfd / 8] |= (1UL << (sockfd % 8));
    _M_event_set.emplace(sockfd, frame);
    _M_count ++;
  }
  COASYNC_ATTRIBUTE((always_inline)) void commit_frame()
  {
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    if(not _M_count)	COASYNC_ATTRIBUTE((likely))
      return;
    int res = ::epoll_wait(mEpoll, _M_event_buf, std::size(_M_event_buf), 0);
    if(res == -1)	COASYNC_ATTRIBUTE((unlikely))
      throw std::system_error(detail::get_errno(), detail::generic_category());
    std::forward_list<std::coroutine_handle<awaitable_frame_base>> outstanding;
    for (int fd {}; fd < res; fd ++)
      {
        auto frame = std::coroutine_handle<awaitable_frame_base>::from_address(static_cast<awaitable_frame_base*>(_M_event_buf[fd].data.ptr));
        outstanding.emplace_front(frame);
      }
    if(alternative_lock.owns_lock()) COASYNC_ATTRIBUTE((likely)) alternative_lock.unlock();
    for(std::coroutine_handle<awaitable_frame_base> frame: outstanding)
      _M_context.push_frame_to_executor(frame);
  }
  COASYNC_ATTRIBUTE((always_inline)) void COASYNC_API cancel_frame(__native_handle sockfd)
  {
    std::coroutine_handle<awaitable_frame_base> deprecated;
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable>
    alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    if((_M_fds_bitmap[sockfd / 8] & (1UL << (sockfd % 8))) == 0) COASYNC_ATTRIBUTE((unlikely))
      return;
    int res = ::epoll_ctl(_M_epollfd, EPOLL_CTL_DEL, sockfd, nullptr);
    if(res == -1)	COASYNC_ATTRIBUTE((unlikely))
      throw std::system_error(detail::get_errno(), detail::generic_category());
    _M_fds_bitmap[sockfd / 8] &= ~(1UL << sockfd % 8);
    deprecated = std::move(_M_event_set[sockfd]);
    _M_count --;
    if(alternative_lock.owns_lock()) COASYNC_ATTRIBUTE((likely)) alternative_lock.unlock();
    _M_context.push_frame_to_executor(deprecated);
  }
private:
  int _M_epollfd;
  ::epoll_event _M_event_buf[64] {0};
  std::unordered_map<int, std::coroutine_handle<awaitable_frame_base>> _M_event_set;
  unsigned long _M_fds_bitmap[512 / 8] {0};
  mutable unsigned long _M_count {0};
  mutable basic_lockable 	_M_lockable;
};
typedef basic_epoll_socket_service<execution_context, true> 		epoll_socketin_service;
typedef basic_epoll_socket_service<execution_context, false> 		epoll_socketout_service;
}
}

#endif // #if defined(__linux__)
#endif
