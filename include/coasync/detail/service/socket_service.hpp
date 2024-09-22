#ifndef COASYNC_SOCKET_SERVICE_INCLUDED
#define COASYNC_SOCKET_SERVICE_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if defined(_WIN32) || defined(_WIN64)
#include "select_socket_service.hpp"
//#include "iocp_socket_service.hpp"
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
typedef select_socketin_service 	socketin_service;
typedef select_socketout_service 	socketout_service;
//typedef iocp_socketin_service 	socketin_service;
//typedef iocp_socketout_service 	socketout_service;

}
}

#elif defined(__linux__) && __has_include(<sys/epoll.h>)
#include "epoll_socket_service.hpp"

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
typedef epoll_socketin_service 		sockin_service;
typedef epoll_socketout_service 	sockout_service;
}
}
#endif


#endif
