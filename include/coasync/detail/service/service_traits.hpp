#ifndef COASYNC_SERVICE_TRAITS_INCLUDED
#define COASYNC_SERVICE_TRAITS_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../config.hpp"
#include <type_traits>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
template <typename service_type, typename = void>
struct has_post_frame_member
  : std::false_type {};
template <typename service_type>
struct has_post_frame_member<service_type, std::void_t<decltype(service_type::post_frame)>>
: std::true_type {};
template <typename service_type>
inline constexpr bool has_post_frame_member_v
  = has_post_frame_member<service_type>::value;

template <typename service_type, typename = void>
struct has_commit_frame_member
  : std::false_type {};
template <typename service_type>
struct has_commit_frame_member<service_type, std::void_t<decltype(service_type::commit_frame)>>
: std::true_type {};
template <typename service_type>
inline constexpr bool has_commit_frame_member_v
  = has_commit_frame_member<service_type>::value;

template <typename service_type, typename = void>
struct has_cancel_frame_member
  : std::false_type {};
template <typename service_type>
struct has_cancel_frame_member<service_type, std::void_t<decltype(service_type::cancel_frame)>>
: std::true_type {};
template <typename service_type>
inline constexpr bool has_cancel_frame_member_v
  = has_cancel_frame_member<service_type>::value;

template <typename service_type>
concept basic_service
  = has_post_frame_member_v<service_type> and has_commit_frame_member_v<service_type>;
template <typename service_type>
concept cancellable_service
  = (basic_service<service_type>) and has_cancel_frame_member_v<service_type>;

template <typename __service_type>
struct service_traits
{
private:
	static_assert(basic_service<__service_type>);
public:
	typedef __service_type service_type;
  typedef typename __service_type::mutex_type mutex_type;
  COASYNC_ATTRIBUTE((nodiscard, always_inline)) static constexpr overlap_arity() noexcept
  {
    return __service_type::overlap_arity();
  }
};
}
}

#endif
