#ifndef __COASYNC_WINDOWS_INITIATE_INCLUDED
#define __COASYNC_WINDOWS_INITIATE_INCLUDED
/// The WSAStartup function must be the first Windows Sockets function called by an application
/// or DLL. It allows an application or DLL to specify the version of Windows Sockets required
/// and retrieve details of the specific Windows Sockets implementation. The application or DLL
/// can only issue further Windows Sockets functions after successfully calling WSAStartup.
#include "config.h"
#if defined(__has_include)
# if defined(_WIN32) && __has_include(<winsock2.h>)
#  include <winsock2.h>
#  if defined(MSVC) && defined (_MSC_VER) && _MSC_VER > 1000
#   pragma comment(lib, "ws2_32.lib")
#  endif
# endif
#endif
#include <cstdio>
#include <utility>
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
/// RAII type for Windows socket library linkage and cleanup
struct windows_initiate final
{
  constexpr windows_initiate& operator=(windows_initiate const&) = delete;
  constexpr windows_initiate& operator=(windows_initiate     &&) = delete;
  COASYNC_ATTRIBUTE((always_inline)) explicit windows_initiate() noexcept
  {
#if defined(_WIN32) || defined(_WIN64)
    COASYNC_ATTRIBUTE((gnu::uninitialized)) WSADATA wsa_data;
    int errc = ::WSAStartup(MAKEWORD(2, 2), std::addressof(wsa_data));
    if (errc == 0) COASYNC_ATTRIBUTE((likely)) return;
    switch (errc)
      {
      case WSASYSNOTREADY:
        std::fprintf(
          stderr, "The underlying network subsystem is not ready for network communication. ");
		 COASYNC_TERMINATE();
      case WSAVERNOTSUPPORTED:
        std::fprintf(
          stderr, "The version of Windows Sockets support requested is not provided "
					"by this particular Windows Sockets implementation. ");
				COASYNC_TERMINATE();
      case WSAEINPROGRESS:
        std::fprintf(
          stderr, "A blocking Windows Sockets 1.1 operation is in progress.");
				COASYNC_TERMINATE();
      case WSAEPROCLIM:
        std::fprintf(
          stderr, "A limit on the number of tasks supported by the Windows Sockets "
					" implementation has been reached. ");
				COASYNC_TERMINATE();
      default:
        std::fprintf(
          stderr, "The lpWSAData parameter is not a valid pointer. ");
				COASYNC_TERMINATE();
      }
    if (LOBYTE(wsa_data.wVersion) != 2 or HIBYTE(wsa_data.wVersion) != 2) [[unlikely]]
      {
        std::fprintf(stderr, "no useable winsock dll");
        ::WSACleanup();
				COASYNC_TERMINATE();
      }
#endif
  }
///An application or DLL is required to perform a successful WSAStartup call before it can
/// use Windows Sockets services. When it has completed the use of Windows Sockets, the application
/// or DLL must call WSACleanup to deregister itself from a Windows Sockets implementation and allow
/// the implementation to free any resources allocated on behalf of the application or DLL.
  COASYNC_ATTRIBUTE((always_inline)) ~ windows_initiate() noexcept
  {
#if defined(_WIN32) || defined(_WIN64)
    ::WSACleanup();
#endif
  }
};
}
}
#endif
