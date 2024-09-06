#ifndef __COASYNC_NETWORKING_INCLUDED
#define __COASYNC_NETWORKING_INCLUDED
#include "config.hpp"
#if defined(__has_include)
# if defined(_WIN32) || defined(_WIN64)
#  if __has_include(<ws2tcpip.h>)
#   include <ws2tcpip.h>
#  endif
# elif defined(__linux__)
#  include <cerrno>
# endif
#endif
/// The error codes returned by Windows Sockets are similar to UNIX socket error code constants,
/// but the constants are all prefixed with WSA. So in Winsock applications the WSAEWOULDBLOCK
/// error code would be returned, while in UNIX applications the EWOULDBLOCK error code
/// would be returned.
#if defined(_WIN32) || defined(_WIN64)
# define get_error_code(error_code) WSA ## error_code
#elif defined(__linux__)
# define get_error_code(error_code) error_code
#endif
#include <system_error>
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
#if defined(_WIN32) || defined(_WIN64)
typedef ::SOCKET 	__native_handle;
#elif defined(__linux__)
typedef int 			__native_handle;
#endif
#if defined(_WIN32) || defined(_WIN64)
COASYNC_ATTRIBUTE((nodiscard, always_inline))
static int get_errno() noexcept
{
/// In Winsock applications, error codes are retrieved using the WSAGetLastError function,
/// the Windows Sockets substitute for the Windows GetLastError function.
  return ::WSAGetLastError();
}
#elif defined(__linux__)
COASYNC_ATTRIBUTE((nodiscard, always_inline))
static /*consteval*/ int get_errno() noexcept
{
  return errno;
}
#endif
COASYNC_ATTRIBUTE((nodiscard, always_inline))
std::error_category const& generic_category() noexcept
{
#if defined(_WIN32) || defined(_WIN64)
  struct WSA_system_category final: public std::error_category
  {
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
		virtual const char* name() const noexcept
    {
      return "wsa_system";
    }
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
		virtual std::string message(int errc) const override
    {
      LPVOID 			buffer;
      std::string result;
/// The WSAGetLastError function returns the last error that occurred for the calling thread.
/// When a particular Windows Sockets function indicates an error has occurred, this function
/// should be called immediately to retrieve the extended error code for the failing function
/// call. These error codes and a short text description associated with an error code are
/// defined in the Winerror.h header file. The FormatMessage function can be used to obtain
/// the message string for the returned error.
      if (DWORD 	length
          = ::FormatMessage(
              FORMAT_MESSAGE_ALLOCATE_BUFFER
              | FORMAT_MESSAGE_FROM_SYSTEM |
              FORMAT_MESSAGE_IGNORE_INSERTS,
              nullptr,
              errc,
              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
              reinterpret_cast<LPTSTR>(std::addressof(
                                         buffer)),
              0,
              nullptr)) COASYNC_ATTRIBUTE((likely))
        {
          result.assign(reinterpret_cast<char const*>(buffer), length * sizeof(TCHAR));
          ::LocalFree(buffer);
          return result;
        }
      else COASYNC_ATTRIBUTE((unlikely)) return "No known";
    }
  };
  static WSA_system_category __cat {};
  return __cat;
#elif defined(__linux__)
  return std::generic_category();
#endif
}
}
}
#endif
