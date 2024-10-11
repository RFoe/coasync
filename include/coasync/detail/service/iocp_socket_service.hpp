#ifndef COASYNC_IOCP_SOCKET_SERVICE_INCLUDED
#define COASYNC_IOCP_SOCKET_SERVICE_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if defined(_WIN32) || defined(_WIN64)

#include "../awaitable_frame_base.hpp"
#include "../basic_lockable.hpp"
#include "../networking.hpp"
#include "../config.hpp"

#include <mutex>
#include <unordered_map>
#include <cstring>
#include <ioapiset.h>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
struct execution_context; // forward declaration
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
template <typename execution_context, bool subcribe_sockin>
struct [[nodiscard]] basic_iocp_socket_service
{
  typedef basic_lockable mutex_type;

  COASYNC_ATTRIBUTE((always_inline)) constexpr explicit basic_iocp_socket_service(execution_context& context) noexcept: _M_context(context)
  {
    _M_iocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    if (_M_iocp == nullptr) COASYNC_ATTRIBUTE((unlikely))
      throw std::system_error(detail::get_errno(), detail::generic_category());
  }
  constexpr basic_iocp_socket_service& operator=(basic_iocp_socket_service const&) = delete;
  constexpr basic_iocp_socket_service(basic_iocp_socket_service const&) = delete;
  COASYNC_ATTRIBUTE((always_inline)) basic_iocp_socket_service(basic_iocp_socket_service&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) basic_iocp_socket_service& operator=(basic_iocp_socket_service&&) noexcept = default;
  COASYNC_ATTRIBUTE((always_inline)) ~ basic_iocp_socket_service() noexcept = default;

  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  static constexpr std::size_t overlap_arity() noexcept
  {
    return 1u;
  }

  COASYNC_ATTRIBUTE((always_inline)) void COASYNC_API post_frame(std::coroutine_handle<awaitable_frame_base> frame, SOCKET sockfd)
  {
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    ::CreateIoCompletionPort(reinterpret_cast<HANDLE>(sockfd), _M_iocp, reinterpret_cast<ULONG_PTR>(sockfd), 0);
    std::memset(std::addressof(_M_overlapped[sockfd]), 0, sizeof(OVERLAPPED));
    _M_overlapped[sockfd].Internal = reinterpret_cast<ULONG_PTR>(frame.address());
    _M_overlapped[sockfd].InternalHigh = reinterpret_cast<ULONG_PTR>(frame.address());
    DWORD flags {};
    if constexpr(subcribe_sockin)
      {
        if(not ::WSARecv(sockfd, nullptr, 0, nullptr, &flags, std::addressof(_M_overlapped[sockfd]), nullptr))
          if(::WSAGetLastError() != ERROR_IO_PENDING)
            throw std::system_error(detail::get_errno(), detail::generic_category());
      }
    else
      {
        if(not ::WSASend(sockfd, nullptr, 0, nullptr, flags, std::addressof(_M_overlapped[sockfd]), nullptr))
          if(::WSAGetLastError() != ERROR_IO_PENDING)
            throw std::system_error(detail::get_errno(), detail::generic_category());
      }
    if(alternative_lock.owns_lock()) COASYNC_ATTRIBUTE((likely)) alternative_lock.unlock();
  }

  COASYNC_ATTRIBUTE((always_inline)) void COASYNC_API commit_frame()
  {
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable> alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    if (not ::GetQueuedCompletionStatus(_M_iocp, &_M_bytes_transferred, &_M_completion_key, reinterpret_cast<LPOVERLAPPED*>(&_M_overlapped[0]), 0))
      COASYNC_ATTRIBUTE((likely))	return;
    auto s = reinterpret_cast<SOCKET>(_M_completion_key);
    auto f = std::coroutine_handle<awaitable_frame_base>::from_address(reinterpret_cast<awaitable_frame_base*>(_M_overlapped[s].Internal));
    std::memset(std::addressof(_M_overlapped[s]), 0, sizeof(OVERLAPPED));
    _M_context.push_frame_to_executor(f);
    if(alternative_lock.owns_lock()) COASYNC_ATTRIBUTE((likely)) alternative_lock.unlock();
  }
  COASYNC_ATTRIBUTE((always_inline)) void COASYNC_API cancel_frame(SOCKET sockfd)
  {
    COASYNC_ATTRIBUTE((maybe_unused)) std::unique_lock<basic_lockable>              alternative_lock;
    if(_M_context.concurrency())  COASYNC_ATTRIBUTE((likely))
      std::unique_lock<basic_lockable>(_M_lockable).swap(alternative_lock);
    if (not ::CancelIoEx(reinterpret_cast<HANDLE>(sockfd), std::addressof(_M_overlapped[sockfd]))) COASYNC_ATTRIBUTE((unlikely))
      throw std::system_error(detail::get_errno(), detail::generic_category());
    auto f = std::coroutine_handle<awaitable_frame_base>::from_address(reinterpret_cast<awaitable_frame_base*>(_M_overlapped[sockfd].Internal));
    _M_overlapped.erase(sockfd);
    _M_context.push_frame_to_executor(f);
    if(alternative_lock.owns_lock()) COASYNC_ATTRIBUTE((likely)) alternative_lock.unlock();
  }
private:
  execution_context& 	_M_context;
  HANDLE 							_M_iocp;
  std::unordered_map<SOCKET, OVERLAPPED> _M_overlapped;
  mutable basic_lockable 	_M_lockable;
  mutable DWORD 		_M_bytes_transferred;
  mutable ULONG_PTR _M_completion_key;
};
typedef basic_iocp_socket_service<execution_context, true> 		iocp_socketin_service;
typedef basic_iocp_socket_service<execution_context, false> 	iocp_socketout_service;
}
}
#endif // #if defined(_WIN32) || defined(_WIN64)
#endif
