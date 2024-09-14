#ifndef COASYNC_AWAITABLE_FRAME_ALLOC_INCLUDED
#define COASYNC_AWAITABLE_FRAME_ALLOC_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "config.hpp"
#if __cpp_impl_coroutine >= 201902 && __cpp_lib_coroutine >= 201902
#  include <coroutine>
#elif defined(__cpp_coroutines) && __has_include(<experimental/coroutine>)
#  include <experimental/coroutine>
namespace std {
using std::experimental::coroutine_handle;
using std::experimental::coroutine_traits;
using std::experimental::noop_coroutine;
using std::experimental::suspend_always;
using std::experimental::suspend_never;
} // namespace std
#else
# error This library requires the use of C++20 coroutine support
#endif
#include <memory>
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
/// Encapsulates strategies for access/addressing, allocation/deallocation and
/// construction/destruction of coroutine objects.
static constexpr std::size_t aligned_allocation_size(std::size_t s, std::size_t a) noexcept
{
  return (s + a - 1) & ~(a - 1);
}
/// https://en.cppreference.com/w/cpp/language/coroutines
/// Coroutine state is allocated dynamically via non-array operator new.
/// If the Promise type defines a class-level replacement, it will be used,
/// otherwise global operator new will be used.
/// If the Promise type defines a placement form of operator new that takes
/// additional parameters, and they match an argument list where the first argument
/// is the size requested (of type std::size_t) and the rest are the coroutine
/// function arguments, those arguments will be passed to operator new
/// (this makes it possible to use leading-allocator-convention for coroutines).
template<typename Alloc>
struct  awaitable_frame_alloc
{
  template<typename... Args>
  static void* operator new (std::size_t frame_size, std::allocator_arg_t, Alloc alloc, Args& ...)
  noexcept(
    noexcept(std::declval<Alloc>().allocate((std::size_t) 0))
    and std::is_nothrow_move_constructible_v<Alloc>)
  {
    void* frame_pointer = alloc.allocate(padded_frame_size(frame_size));
    void* allocator_pointer = std::addressof(get_allocator(frame_pointer, frame_size));
    ::new (allocator_pointer) Alloc(std::move(alloc));
    return frame_pointer;
  }
  template<typename This, typename... Args>
  static void* operator new (std::size_t frame_size, This&, std::allocator_arg_t, Alloc alloc, Args& ...)
  noexcept(
    noexcept(std::declval<Alloc>().allocate((std::size_t) 0))
    and std::is_nothrow_move_constructible_v<Alloc>)
  {
    return awaitable_frame_alloc::operator new (frame_size, std::allocator_arg, std::move(alloc));
  }
  static void operator delete (void* ptr, std::size_t frame_size)
  noexcept(
    noexcept(std::declval<Alloc>().deallocate((std::byte*)0, (std::size_t) 0))
    and std::is_nothrow_destructible_v<Alloc>)
  {
    Alloc& alloc = get_allocator(ptr, frame_size);
    Alloc local_alloc(std::move(alloc));
    alloc.~Alloc();
    local_alloc.deallocate(static_cast<std::byte*>(ptr), padded_frame_size(frame_size));
  }
private:
  static std::size_t offset_of_allocator(std::size_t frame_size) noexcept
  {
    return aligned_allocation_size(frame_size, alignof(Alloc));
  }
  static std::size_t padded_frame_size(std::size_t frame_size) noexcept
  {
    return offset_of_allocator(frame_size) + sizeof(Alloc);
  }
  static Alloc& get_allocator(void* frame_pointer, std::size_t frame_size) noexcept
  {
    char* allocator_pointer = static_cast<char*>(frame_pointer) + offset_of_allocator(frame_size);
#if defined(__cpp_lib_launder)
    return *std::launder(reinterpret_cast<Alloc*>(allocator_pointer));
#else
    return *reinterpret_cast<Alloc*>(allocator_pointer);
#endif
  }
};
template <typename Alloc>
static bool
constexpr allocator_needs_to_be_stored
  = not std::allocator_traits<Alloc>::is_always_equal::value
    or not std::is_default_constructible_v<Alloc>;
template<typename Alloc>
requires (not allocator_needs_to_be_stored<Alloc>)
struct  awaitable_frame_alloc<Alloc>
{
  static void* operator new (std::size_t frame_size)
  noexcept(
    noexcept(std::declval<Alloc>().allocate((std::size_t) 0))
    and std::is_nothrow_default_constructible_v<Alloc>)
  {
    Alloc alloc;
    return alloc.allocate(frame_size);
  }
  static void operator delete (void* frame_pointer, std::size_t frame_size)
  noexcept(
    noexcept(std::declval<Alloc>().deallocate((std::byte*)0, (std::size_t) 0))
    and std::is_nothrow_destructible_v<Alloc>)
  {
    Alloc alloc;
    alloc.deallocate(static_cast<std::byte*>(frame_pointer), frame_size);
  }
};
}
}
#endif
