#ifndef COASYNC_SCOPE_ID_INCLUDED
#define COASYNC_SCOPE_ID_INCLUDED
#include "../detail/config.h"
#include <cstdint>
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
typedef std::uint_least32_t scope_id_type;
}
}
#endif
