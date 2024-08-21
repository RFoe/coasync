#ifndef __COASYNC_PORT_INCLUDED
#define __COASYNC_PORT_INCLUDED
#include "../detail/config.h"
#include <cstdint>
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
typedef std::uint_least16_t port_type;
}
}
#endif
