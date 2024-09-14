#ifndef COASYNC_SCOPE_ID_INCLUDED
#define COASYNC_SCOPE_ID_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../detail/config.hpp"
#include <cstdint>
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
typedef std::uint_least32_t scope_id_type;
}
}
#endif
