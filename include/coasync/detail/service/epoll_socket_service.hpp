#ifndef COASYNC_EPOLL_SOCKET_SERVICE_INCLUDED
#define COASYNC_EPOLL_SOCKET_SERVICE_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../config.hpp"

#if defined(__linux__) && __has_include(<sys/epoll.h>)

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{

}
}

#endif // #if defined(__linux__)
#endif
