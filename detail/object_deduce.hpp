#ifndef COASYNC_OBJECT_DEDUCE_INCLUDED
#define COASYNC_OBJECT_DEDUCE_INCLUDED
#include "config.h"
#include <type_traits>
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
struct void_t {};
/// Type purification to remove references and void types so that
/// they can be stored in heterogeneous containers(std::tuple/std::variant)
/// heterogeneous value constructor is defined as deleted if the initialization of any element that is a reference
template <typename T>
using object_deduce_t = std::conditional_t<std::is_void_v<T>, void_t, std::decay_t<T>>;
}
}
#endif
