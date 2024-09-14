#ifndef COASYNC_FUNCTION_TRAITS_INCLUDED
#define COASYNC_FUNCTION_TRAITS_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../config.hpp"
#include <functional>

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
struct unit {};
/// requirement: The compiler supports partial specialization of class templates.
/// The template argument F is a function type
template <typename F> struct function_traits: std::false_type {};
/// remove cv-qualifiers
template <typename F> requires (not std::same_as<std::remove_cvref_t<F>, F>)
struct function_traits<F> : function_traits<std::remove_cvref_t<F>> {};
/// invocable archetype for function_traits
template <typename R, typename... Args>
struct function_traits<R(Args ...)>: std::true_type
{
  inline static constexpr std::size_t arity = { sizeof...(Args) };
  /// An integral constant expression that gives the number of arguments accepted by the function type F.
  typedef R 																		result;
  /// The type returned by function type F.
  typedef std::tuple<std::decay_t<Args> ...> 		tuple;
  /// the tuple type of argument types
  using function = 															R(Args ...);
  using pointer = 															R(*)(Args ...);
  template <std::size_t I>
  struct argument
  {
    static_assert(I < sizeof...(Args));
    typedef typename std::tuple_element<I, std::tuple<Args ...>>::type type;
  };
  template <std::size_t I>
  using argument_t = typename argument<I>::type;
  /// The Nth argument type of function type F, where 1 <= N <= arity of F.
};
/// R(Args ...) const/volatile/noexcept
template <typename R, typename... Args>
struct function_traits<R(Args ...) const> : function_traits<R(Args ...)>
{
  inline static constexpr unit is_const {};
};
template <typename R, typename... Args>
struct function_traits<R(Args ...) volatile> : function_traits<R(Args ...)>
{
  inline static constexpr unit is_volatile {};
};
template <typename R, typename... Args>
struct function_traits<R(Args ...) noexcept> : function_traits<R(Args ...)>
{
  inline static constexpr unit is_noexcept {};
};
template <typename R, typename... Args>
struct function_traits<R(Args ...) const volatile> : function_traits<R(Args ...)>
{
  inline static constexpr unit is_const {};
  inline static constexpr unit is_volatile {};
};
template <typename R, typename... Args>
struct function_traits<R(Args ...) const noexcept> : function_traits<R(Args ...)>
{
  inline static constexpr unit is_const {};
  inline static constexpr unit is_noexcept {};
};
template <typename R, typename... Args>
struct function_traits<R(Args ...) volatile noexcept> : function_traits<R(Args ...)>
{
  inline static constexpr unit is_volatile {};
  inline static constexpr unit is_noexcept {};
};
template <typename R, typename... Args>
struct function_traits<R(Args ...) const volatile noexcept> : function_traits<R(Args ...)>
{
  inline static constexpr unit is_const {};
  inline static constexpr unit is_volatile {};
  inline static constexpr unit is_noexcept {};
};
/// function pointer
template <typename R, typename... Args>
struct function_traits<R(*)(Args ...)> : function_traits<R(Args ...)> {};
template <typename R, typename... Args>
struct function_traits<R(*)(Args ...) noexcept> : function_traits<R(Args ...)>
{
  inline static constexpr unit is_noexcept {};
};
/// std::function
template <typename R, typename... Args>
struct function_traits<std::function<R(Args ...)>> : function_traits<R(Args ...)> {};
/// std::reference_wrapper
template <typename R, typename... Args>
struct function_traits<std::reference_wrapper<R(Args ...)>> : function_traits<R(Args ...)> {};
/// member function pointer
template <typename R, typename Class, typename... Args>
struct function_traits<R(Class::*)(Args ...)> : function_traits<R(Args ...)>
{
  using pointer = R(Class::*)(Args ...);
  typedef Class class_type;
  inline static constexpr unit is_member {};
};
template <typename R, typename Class, typename... Args>
struct function_traits<R(Class::*)(Args ...) const> : function_traits<R(Class::*)(Args ...)>
{
  inline static constexpr unit is_const {};
};
template <typename R, typename Class, typename... Args>
struct function_traits<R(Class::*)(Args ...) volatile> : function_traits<R(Class::*)(Args ...)>
{
  inline static constexpr unit is_volatile {};
};
template <typename R, typename Class, typename... Args>
struct function_traits<R(Class::*)(Args ...) noexcept> : function_traits<R(Class::*)(Args ...)>
{
  inline static constexpr unit is_noexcept {};
};
template <typename R, typename Class, typename... Args>
struct function_traits<R(Class::*)(Args ...) const volatile> : function_traits<R(Class::*)(Args ...)>
{
  inline static constexpr unit is_const {};
  inline static constexpr unit is_volatile {};
};
template <typename R, typename Class, typename... Args>
struct function_traits<R(Class::*)(Args ...) const noexcept> : function_traits<R(Class::*)(Args ...)>
{
  inline static constexpr unit is_const {};
  inline static constexpr unit is_noexcept {};
};
template <typename R, typename Class, typename... Args>
struct function_traits<R(Class::*)(Args ...) volatile noexcept> : function_traits<R(Class::*)(Args ...)>
{
  inline static constexpr unit is_volatile {};
  inline static constexpr unit is_noexcept {};
};
template <typename R, typename Class, typename... Args>
struct function_traits<R(Class::*)(Args ...) const volatile noexcept> : function_traits<R(Class::*)(Args ...)>
{
  inline static constexpr unit is_const {};
  inline static constexpr unit is_volatile {};
  inline static constexpr unit is_noexcept {};
};
/// functors
template <typename Functor> requires requires { &Functor::operator(); }
struct function_traits<Functor> : function_traits<decltype(&Functor::operator())>
{
  inline static constexpr unit is_functor {};
};
/// traits
/// see: https://en.cppreference.com/w/cpp/types/void_t
/// Utility metafunction that maps a sequence of any types to the type void.
/// This metafunction is a convenient way to leverage SFINAE prior to C++20's concepts,
/// in particular for conditionally removing functions from the candidate set based
/// on whether an expression is valid in the unevaluated context (such as operand to
/// decltype expression), allowing to exist separate function overloads or specializations
/// based on supported operations.
template <typename F, typename = void>
struct is_const_function : std::false_type {};
template <typename F>
struct is_const_function<F, std::void_t<decltype(F::traits_type::is_const)>> : std::true_type {};
template <typename F>
inline constexpr bool is_const_function_v = is_const_function<F>::value;
template <typename F> concept const_function = is_const_function_v<F>;
// const_function
template <typename F, typename = void>
struct is_volatile_function : std::false_type {};
template <typename F>
struct is_volatile_function<F, std::void_t<decltype(F::traits_type::is_volatile)>> : std::true_type {};
template <typename F>
inline constexpr bool is_volatile_function_v = is_volatile_function<F>::value;
template <typename F> concept volatile_function = is_volatile_function_v<F>;
// volatile_function
template <typename F, typename = void>
struct is_noexcept_function : std::false_type {};
template <typename F>
struct is_noexcept_function<F, std::void_t<decltype(F::traits_type::is_noexcept)>> : std::true_type {};
template <typename F>
inline constexpr bool is_noexcept_function_v = is_noexcept_function<F>::value;
template <typename F> concept noexcept_function = is_noexcept_function_v<F>;
// noexcept_function
template <typename F, typename = void>
struct is_functor_function : std::false_type {};
template <typename F>
struct is_functor_function<F, std::void_t<decltype(F::traits_type::is_functor)>> : std::true_type {};
template <typename F>
inline constexpr bool is_functor_function_v = is_functor_function<F>::value;
template <typename F> concept functor_function = is_functor_function_v<F>;
}
}
#endif
