#ifndef COASYNC_CONFIG_INCLUDED
#define COASYNC_CONFIG_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

/// Check the compilation environment

#if __cplusplus < 202002L
#  if defined(_MSC_VER) && !defined(__clang__)
//#    error This library requires the use of C++20. Use /Zc:__cplusplus to enable __cplusplus conformance.
#  else
#    error This library requires the use of C++20.
#  endif
#endif

#if defined(_MSC_VER) && !defined(__clang__) && (!defined(_MSVC_TRADITIONAL) || _MSVC_TRADITIONAL)
//#  error This library requires the use of the new conforming preprocessor enabled by /Zc:preprocessor.
#endif

/// The standard defines a set of preprocessor macros corresponding to C++ language and
/// library features introduced in C++11 or later. They are intended as a simple and
/// portable way to detect the presence of said features.
#if __has_include(<version>)
#  include <version>
#else
/// This header was originally in the C standard library as <iso646.h>.
/// Compatibility header, in C defines alternative operator representations which are keywords in C++.
/// This means that in a conforming implementation, including this header has no effect.
#  include <ciso646> // For stdlib feature-test macros when <version> is not available
#endif

#include <cassert>
#include <type_traits>
#include <exception>

/// Introduces implementation-defined attributes for types, objects, code, etc.
/// provide the unified standard syntax for implementation-defined language extensions,
/// such as the GNU and IBM language extensions __attribute__((...)), Microsoft extension __declspec(), etc.

#define COASYNC_COUNT(...) _COASYNC_COUNT_INTERNAL(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define _COASYNC_COUNT_INTERNAL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _No, ...) _No
#define COASYNC_CONCAT(_A, _B) _A ## _B

#define COASYNC_CAT_(_XP, ...)    _XP##__VA_ARGS__
#define COASYNC_CAT(_XP, ...)     COASYNC_CAT_(_XP, __VA_ARGS__)

#define COASYNC_EXPAND(...)       __VA_ARGS__
#define COASYNC_EAT(...)

#define COASYNC_CHECK(...)                                                COASYNC_EXPAND(COASYNC_CHECK_(__VA_ARGS__, 0, ))
#define COASYNC_CHECK_(_XP, _NP, ...)                                     _NP
#define COASYNC_PROBE(...)                                                COASYNC_PROBE_(__VA_ARGS__, 1)
#define COASYNC_PROBE_(_XP, _NP, ...)                                     _XP, _NP,

#define COASYNC_EXPAND_R(...)                                                                      \
  COASYNC_EXPAND_R1(COASYNC_EXPAND_R1(COASYNC_EXPAND_R1(COASYNC_EXPAND_R1(__VA_ARGS__))))          \
  /**/
#define COASYNC_EXPAND_R1(...)                                                                     \
  COASYNC_EXPAND_R2(COASYNC_EXPAND_R2(COASYNC_EXPAND_R2(COASYNC_EXPAND_R2(__VA_ARGS__))))          \
  /**/
#define COASYNC_EXPAND_R2(...)                                                                     \
  COASYNC_EXPAND_R3(COASYNC_EXPAND_R3(COASYNC_EXPAND_R3(COASYNC_EXPAND_R3(__VA_ARGS__))))          \
  /**/
#define COASYNC_EXPAND_R3(...)                                                                     \
  COASYNC_EXPAND(COASYNC_EXPAND(COASYNC_EXPAND(COASYNC_EXPAND(__VA_ARGS__))))                      \
  /**/
#define COASYNC_PARENS ()
/// token sequence __VA_OPT__(content??), which is replaced by content if __VA_ARGS__
/// is non-empty, and expands to nothing otherwise.
#define COASYNC_FOR_EACH(_MACRO, ...)                                                              \
  __VA_OPT__(COASYNC_EXPAND_R(COASYNC_FOR_EACH_HELPER(_MACRO, __VA_ARGS__)))                       \
  /**/
#define COASYNC_FOR_EACH_HELPER(_MACRO, _A1, ...)                                                  \
  _MACRO(_A1) __VA_OPT__(COASYNC_FOR_EACH_AGAIN COASYNC_PARENS(_MACRO, __VA_ARGS__)) /**/
#define COASYNC_FOR_EACH_AGAIN()       COASYNC_FOR_EACH_HELPER

#define COASYNC_HEAD_OR_TAIL(_XP, ...) COASYNC_EXPAND __VA_OPT__((__VA_ARGS__) COASYNC_EAT)(_XP)
#define COASYNC_HEAD_OR_NULL(_XP, ...) COASYNC_EXPAND __VA_OPT__(() COASYNC_EAT)(_XP)

#if defined(__NVCC__)
#  define COASYNC_NVCC(...) COASYNC_HEAD_OR_TAIL(1, __VA_ARGS__)
#elif defined(__NVCOMPILER)
#  define COASYNC_NVHPC(...) COASYNC_HEAD_OR_TAIL(1, __VA_ARGS__)
#elif defined(__EDG__)
#  define COASYNC_EDG(...) COASYNC_HEAD_OR_TAIL(1, __VA_ARGS__)
#elif defined(__clang__)
#  define COASYNC_CLANG(...) COASYNC_HEAD_OR_TAIL(1, __VA_ARGS__)
#  if defined(_MSC_VER)
#    define COASYNC_CLANG_CL(...) COASYNC_HEAD_OR_TAIL(1, __VA_ARGS__)
#  endif
#  if defined(__apple_build_version__)
#    define COASYNC_APPLE_CLANG(...) COASYNC_HEAD_OR_TAIL(1, __VA_ARGS__)
#  endif
#elif defined(__GNUC__)
#  define COASYNC_GCC(...) COASYNC_HEAD_OR_TAIL(1, __VA_ARGS__)
#elif defined(_MSC_VER)
#  define COASYNC_MSVC(...) COASYNC_HEAD_OR_TAIL(1, __VA_ARGS__)
#endif

#ifndef COASYNC_NVCC
#  define COASYNC_NVCC(...) COASYNC_HEAD_OR_NULL(0, __VA_ARGS__)
#endif
#ifndef COASYNC_NVHPC
#  define COASYNC_NVHPC(...) COASYNC_HEAD_OR_NULL(0, __VA_ARGS__)
#endif
#ifndef COASYNC_EDG
#  define COASYNC_EDG(...) COASYNC_HEAD_OR_NULL(0, __VA_ARGS__)
#endif
#ifndef COASYNC_CLANG
#  define COASYNC_CLANG(...) COASYNC_HEAD_OR_NULL(0, __VA_ARGS__)
#endif
#ifndef COASYNC_CLANG_CL
#  define COASYNC_CLANG_CL(...) COASYNC_HEAD_OR_NULL(0, __VA_ARGS__)
#endif
#ifndef COASYNC_APPLE_CLANG
#  define COASYNC_APPLE_CLANG(...) COASYNC_HEAD_OR_NULL(0, __VA_ARGS__)
#endif
#ifndef COASYNC_GCC
#  define COASYNC_GCC(...) COASYNC_HEAD_OR_NULL(0, __VA_ARGS__)
#endif
#ifndef COASYNC_MSVC
#  define COASYNC_MSVC(...) COASYNC_HEAD_OR_NULL(0, __VA_ARGS__)
#endif

#ifdef __CUDACC__
#  define COASYNC_CUDA(...) COASYNC_HEAD_OR_TAIL(1, __VA_ARGS__)
#else
#  define COASYNC_CUDA(...) COASYNC_HEAD_OR_NULL(0, __VA_ARGS__)
#endif

#define COASYNC_ATTRIBUTE(_XP) COASYNC_FOR_EACH(COASYNC_ATTR, COASYNC_EXPAND _XP)
#define COASYNC_ATTR(_ATTR)                                                                        \
  COASYNC_CAT(COASYNC_ATTR_WHICH_, COASYNC_CHECK(COASYNC_CAT(COASYNC_ATTR_, _ATTR)))(_ATTR)

#define COASYNC_ATTR_WHICH_0(_ATTR) [[_ATTR]]

#ifdef __CUDACC__
#  define COASYNC_ATTR_WHICH_1(_ATTR) __host__
#else
#  define COASYNC_ATTR_WHICH_1(_ATTR)
#endif
#define COASYNC_ATTR_host     COASYNC_PROBE(~, 1)
#define COASYNC_ATTR___host__ COASYNC_PROBE(~, 1)

#ifdef __CUDACC__
#  define COASYNC_ATTR_WHICH_2(_ATTR) __device__
#else
#  define COASYNC_ATTR_WHICH_2(_ATTR)
#endif
#define COASYNC_ATTR_device     COASYNC_PROBE(~, 2)
#define COASYNC_ATTR___device__ COASYNC_PROBE(~, 2)

#if COASYNC_NVHPC()
#  define COASYNC_ATTR_WHICH_3(_ATTR) /*nothing*/
#elif COASYNC_MSVC()
#  define COASYNC_ATTR_WHICH_3(_ATTR) [[msvc::no_unique_address]]
#elif COASYNC_CLANG_CL()
#  define COASYNC_ATTR_WHICH_3(_ATTR) [[msvc::no_unique_address]]
#else
#  define COASYNC_ATTR_WHICH_3(_ATTR) [[no_unique_address]]
#endif
#define COASYNC_ATTR_no_unique_address COASYNC_PROBE(~, 3)

#if COASYNC_MSVC()
#  define COASYNC_ATTR_WHICH_4(_ATTR) __forceinline
#elif COASYNC_CLANG()
#  define COASYNC_ATTR_WHICH_4(_ATTR)                                                              \
    __attribute__((__always_inline__, __artificial__, __nodebug__)) inline
#elif defined(__GNUC__)
#  define COASYNC_ATTR_WHICH_4(_ATTR) __attribute__((__always_inline__, __artificial__)) inline
#else
#  define COASYNC_ATTR_WHICH_4(_ATTR) /*nothing*/
#endif
#define COASYNC_ATTR_always_inline COASYNC_PROBE(~, 4)

#if COASYNC_CLANG() || COASYNC_GCC()
#  define COASYNC_ATTR_WHICH_5(_ATTR) __attribute__((__weak__))
#else
#  define COASYNC_ATTR_WHICH_5(_ATTR) /*nothing*/
#endif
#define COASYNC_ATTR_weak     COASYNC_PROBE(~, 5)
#define COASYNC_ATTR___weak__ COASYNC_PROBE(~, 5)

/// the compiler could use the extended instruction sets even if the built-ins
/// are not used explicitly in the program. For this reason, applications that
/// perform run-time CPU detection must compile separate files for each supported
/// architecture, using the appropriate flags. In particular, the file containing
/// the CPU detection code should be compiled without these options.
#if !COASYNC_MSVC() && defined(__has_builtin)
#  define COASYNC_HAS_BUILTIN __has_builtin
#else
#  define COASYNC_HAS_BUILTIN(...) 0
#endif

#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
#  define COASYNC_UNREACHABLE() std::unreachable()
#elif COASYNC_HAS_BUILTIN(__builtin_unreachable)
#  define COASYNC_UNREACHABLE() __builtin_unreachable()
#elif COASYNC_MSVC()
#  define COASYNC_UNREACHABLE(...) __assume(false)
#else
#  define COASYNC_UNREACHABLE(...) std::terminate()
#endif

#if COASYNC_NVHPC()
#  include <nv/target>
#  define COASYNC_TERMINATE() NV_IF_TARGET(NV_IS_HOST, (std::terminate();), (__trap();)) void()
#elif COASYNC_CLANG() && COASYNC_CUDA() && defined(__CUDA_ARCH__)
#  define COASYNC_TERMINATE()                                                                      \
    __trap();                                                                                      \
    __builtin_unreachable()
#else
#  define COASYNC_TERMINATE() std::terminate()
#endif

#define COASYNC_ASSERT(_XP)                                                                        \
  do {                                                                                             \
    static_assert(noexcept(_XP));                                                                  \
    assert(_XP);                                                                        \
  } while (false)

#define COASYNC_API

#endif
