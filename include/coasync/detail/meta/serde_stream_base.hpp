#ifndef COASYNC_SERDE_STREAM_BASE_INCLUDED
#define COASYNC_SERDE_STREAM_BASE_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "../../awaitable.hpp"
#include "../variant"
#include "member_field.hpp"
#include "signature.hpp"
#include "magic_get.hpp"
#include <typeindex>
#include <stack>
#include <queue>
#include <limits>

///  serialization library.
/// takes arbitrary data types and reversibly turns them into different
/// representations [compact binary encodings]
/// designed to be fast, light-weight, and easy to extend - it has no
/// external dependencies and can be easily bundled with other code or used standalone.
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
/// The Curiously Recurring Template Pattern is an idiom in which a class X derives from
/// a class template Y, taking a template parameter Z, where Y is instantiated with Z = X.
/// CRTP may be used to implement "compile-time polymorphism", when a base class exposes
/// an interface, and derived classes implement such interface.
template <typename crtp_object> struct serde_stream_base
{
/// requirement for crtp_object type
///	 	awaitable<void> write(char const*, std::streamsize)
///   awaitable<void> read(char *, std::streamsize)
private:
  /// Message Structure: Tag Size[optional] Value
  /// Algorithm: top-down parsing
  typedef unsigned short desc_t;
  /// Invoke the [de]serialize with the elements of a tuple as arguments.
  /// Exposition-only function [de]serialize-impl defined as follows:
  template <typename Tuple, std::size_t... Indices>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> _M_do_serialize_tuple_impl(
    Tuple const& t,
    COASYNC_ATTRIBUTE((maybe_unused)) std::index_sequence<Indices ...>)
  {
    ((co_await serialize(std::get<Indices>(t))), ...);
  }
  template <typename Tuple>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> _M_do_serialize_tuple(Tuple const& t)
  {
    co_await _M_do_serialize_tuple_impl(t, std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>> {});
  }
  template <typename T, typename Fields, std::size_t... Indices>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> _M_do_serialize_field_impl(
    T const& 			value,
    Fields const& fields,
    COASYNC_ATTRIBUTE((maybe_unused)) std::index_sequence<Indices ...>)
  {
    ((co_await serialize(value.*(std::get<Indices>(fields).pointer()))), ...);
  }
  template <typename T, typename Fields>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> _M_do_serialize_field(T const& value, Fields const& fields)
  {
    co_await _M_do_serialize_tuple_impl(value, fields, std::make_index_sequence<std::tuple_size_v<std::decay_t<Fields>>> {});
  }
  template <typename Aggr, std::size_t... Indices>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> _M_do_serialize_aggregate_impl(
    Aggr const& t,
    COASYNC_ATTRIBUTE((maybe_unused)) std::index_sequence<Indices ...>)
  {
    ((co_await serialize(magic_get<sizeof...(Indices), Indices>(t))), ...);
  }
  template <typename Aggr>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> _M_do_serialize_aggregate(Aggr const& t)
  {
    co_await _M_do_serialize_aggregate_impl(t, std::make_index_sequence<struct_arity<Aggr>()> {});
  }
  template <typename Tuple, std::size_t... Indices>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> _M_do_deserialize_tuple_impl(
    Tuple& t,
    COASYNC_ATTRIBUTE((maybe_unused)) std::index_sequence<Indices ...>)
  {
    ((co_await deserialize(std::get<Indices>(t))), ...);
  }
  template <typename Tuple>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> _M_do_deserialize_tuple(Tuple& t)
  {
    co_await _M_do_deserialize_tuple_impl(t, std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>> {});
  }
  template <typename T, typename Fields, std::size_t... Indices>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> _M_do_deserialize_field_impl(T& value, Fields const& f, COASYNC_ATTRIBUTE((maybe_unused)) std::index_sequence<Indices ...>)
  {
    ((co_await deserialize(value.*(std::get<Indices>(f).pointer()))), ...);
  }
  template <typename T, typename Fields>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> _M_do_deserialize_field(T& value, Fields const& f)
  {
    co_await _M_do_serialize_tuple_impl(value, f, std::make_index_sequence<std::tuple_size_v<std::decay_t<Fields>>> {});
  }
  template <typename Aggr, std::size_t... Indices>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> _M_do_deserialize_aggregate_impl(
    Aggr& t,
    COASYNC_ATTRIBUTE((maybe_unused)) std::index_sequence<Indices ...>)
  {
    ((co_await deserialize(magic_get<sizeof...(Indices), Indices>(t))), ...);
  }
  template <typename Aggr>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> _M_do_deserialize_aggregate(Aggr& t)
  {
    co_await _M_do_deserialize_aggregate_impl(t, std::make_index_sequence<struct_arity<Aggr>()> {});
  }
  ///  container adaptor : The class template acts as a wrapper to the underlying container
  template <typename T, typename Container = std::deque<T>>
  struct __stack_base: std::stack<T, Container>
  {
    typedef typename Container::const_iterator iterator;
    typedef typename Container::size_type size_type;
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    iterator begin() const noexcept
    {
      return this->c.cbegin();
    }
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    iterator end() const noexcept
    {
      return this->c.cend();
    }
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    size_type size() const noexcept
    {
      return this->c.size();
    }
  };
  template <typename T, typename Containter = std::deque<T>>
  struct __priority_queue_base: std::priority_queue<T, Containter>
  {
    typedef typename Containter::const_iterator iterator;
    typedef typename Containter::size_type size_type;
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    iterator begin() const noexcept
    {
      return this->c.cbegin();
    }
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    iterator end() const noexcept
    {
      return this->c.cend();
    }
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    size_type size() const noexcept
    {
      return this->c.size();
    }
  };
  template <typename T, typename Container = std::deque<T>>
  struct __queue_base: std::queue<T, Container>
  {
    typedef typename Container::const_iterator iterator;
    typedef typename Container::size_type size_type;
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    iterator begin() const noexcept
    {
      return this->c.cbegin();
    }
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    iterator end() const noexcept
    {
      return this->c.cend();
    }
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    size_type size() const noexcept
    {
      return this->c.size();
    }
  };
  struct customized_tag_t {} customized_tag;
  struct smallint_tag_t {} smallint_tag;
  struct plain_tag_t {} plain_tag;
  struct range_tag_t {} range_tag;
  struct aggregate_tag_t {} aggregate_tag;
  template <typename> struct non_linear_container: std::false_type {};
  template <typename T, typename Container>
  struct non_linear_container<std::stack<T, Container>>: std::true_type {};
  template <typename T, typename Container>
  struct non_linear_container<std::priority_queue<T, Container>>: std::true_type {};
  template <typename T, typename Container>
  struct non_linear_container<std::queue<T, Container>>: std::true_type {};
  template <typename...Args>
  struct non_linear_container<std::tuple<Args ...>>: std::true_type {};
  template <typename T, typename K>
  struct non_linear_container<std::pair<T, K>>: std::true_type {};
public:
  constexpr awaitable<void> operator=(serde_stream_base const&) = delete;
  /// POD (Plain Old Data) is a concept in C++ that refers to a type of data that can be copied
  /// and transferred through simple memory replication. Pod-type objects can be copied by
  /// memcpy or other equivalent operations, and their memory layout is completely
  /// transparent and predictable.
  template <typename T>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API serialize(T const& value)
  {
    if constexpr(meta<T>::value)
        return serialize_impl(customized_tag, value);
    else if constexpr(std::integral<T> and sizeof(T) > 2)
        return serialize_impl(smallint_tag, value);
    else if constexpr(std::is_standard_layout_v<T> and std::is_trivial_v<T>)
        return serialize_impl(plain_tag, value);
    else if constexpr(std::ranges::sized_range<T>)
        return serialize_impl(range_tag, value);
    else if constexpr(non_linear_container<T>::value)
        return serialize_impl(value);
    else if constexpr(std::is_aggregate_v<T>)
        return serialize_impl(aggregate_tag, value);
  }
  template <typename T>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API deserialize(T& value)
  {
    if constexpr(meta<T>::value)
        return deserialize_impl(customized_tag, value);
    else if constexpr(std::integral<T> and sizeof(T) > 2)
        return deserialize_impl(smallint_tag, value);
    else if constexpr(std::is_standard_layout_v<T> and std::is_trivial_v<T>)
        return deserialize_impl(plain_tag, value);
    else if constexpr(std::ranges::sized_range<T>)
        return deserialize_impl(range_tag, value);
    else if constexpr(non_linear_container<T>::value)
        return deserialize_impl(value);
    else if constexpr(std::is_aggregate_v<T>)
        return deserialize_impl(aggregate_tag, value);
  }
protected:
  template <typename T>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API serialize_impl(COASYNC_ATTRIBUTE((maybe_unused)) smallint_tag_t, T const& value)
  {
    COASYNC_ATTRIBUTE((gnu::uninitialized)) std::make_unsigned_t<T> uvalue;
    if constexpr(std::unsigned_integral<T>) uvalue = value;
    else uvalue = static_cast<std::make_unsigned_t<T>>((value << 1) ^ (value >> (std::numeric_limits<T>::digits - 1)));
    while (uvalue >= 0x80)
      {
        auto b = static_cast<std::uint8_t>(uvalue) | 0x80;
        co_await static_cast<crtp_object&>(*this)
        .write(reinterpret_cast<typename crtp_object::char_type const*>(&b), sizeof(std::uint8_t));
        uvalue >>= 7;
      }
    auto b = static_cast<std::uint8_t>(uvalue);
    co_await static_cast<crtp_object&>(*this)
    .write(reinterpret_cast<typename crtp_object::char_type const*>(&b), sizeof(std::uint8_t));
  }

  template <typename T>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API serialize_impl(COASYNC_ATTRIBUTE((maybe_unused)) plain_tag_t, T const& value)
  {
    if(not _M_range_dependent) COASYNC_ATTRIBUTE((likely))
      co_await _M_put_desc(_S_condense(std::type_index{ typeid(T) }.hash_code()));
    co_await static_cast<crtp_object&>(*this)
    .write(reinterpret_cast<typename crtp_object::char_type const*>(&value), sizeof(T));
  }
  /// The sized_range concept specifies the requirements of a range type that
  /// knows its size in constant time with the size function.
  template <typename Rng>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API serialize_impl(COASYNC_ATTRIBUTE((maybe_unused)) range_tag_t,Rng const& value)
  /// std::array is both pod-type and fits sized_range concept
  {
    if(not _M_range_dependent) COASYNC_ATTRIBUTE((likely))
      co_await _M_put_desc(_S_condense(std::type_index{ typeid(Rng) }.hash_code()));
    bool const previous_state = _M_range_dependent;
    _M_range_dependent = true;
    co_await _M_put_desc(_S_condense(std::ranges::size(value)));
    for(auto const& element: value) co_await serialize(element);
    _M_range_dependent = previous_state;
  }
  /// sequence container adapter as follow
  template <typename T, typename Container>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API serialize_impl(std::stack<T, Container> const& value)
  {
    if(not _M_range_dependent) COASYNC_ATTRIBUTE((likely))
      co_await _M_put_desc(_S_condense(std::type_index{ typeid(std::stack<T, Container>) }.hash_code()));
    bool const previous_state = _M_range_dependent;
    _M_range_dependent = true;
    __stack_base<T, Container> const& value_base = static_cast<__stack_base<T, Container> const&>(value);
    co_await _M_put_desc(_S_condense(std::ranges::size(value_base)));
    for(auto const& element: value_base) co_await serialize(element);
    _M_range_dependent = previous_state;
  }
  template <typename T, typename Container>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API serialize_impl(std::priority_queue<T, Container> const& value)
  {
    if(not _M_range_dependent) COASYNC_ATTRIBUTE((likely))
      co_await _M_put_desc(_S_condense(std::type_index{ typeid(std::priority_queue<T, Container>) }.hash_code()));
    bool const previous_state = _M_range_dependent;
    _M_range_dependent = true;
    __stack_base<T, Container> const& value_base = static_cast<__priority_queue_base<T, Container> const&>(value);
    co_await _M_put_desc(_S_condense(std::ranges::size(value_base)));
    for(auto const& element: value_base) co_await serialize(element);
    _M_range_dependent = previous_state;
  }
  template <typename T, typename Container>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API serialize_impl(std::queue<T, Container> const& value)
  {
    if(not _M_range_dependent) COASYNC_ATTRIBUTE((likely))
      co_await _M_put_desc(_S_condense(std::type_index{ typeid(std::queue<T, Container>) }.hash_code()));
    bool const previous_state = _M_range_dependent;
    _M_range_dependent = true;
    __queue_base<T, Container> const& value_base = static_cast<__queue_base<T, Container> const&>(value);
    co_await _M_put_desc(_S_condense(std::ranges::size(value_base)));
    for(auto const& element: value_base) co_await serialize(element);
    _M_range_dependent = previous_state;
  }
  /// Class template std::tuple/std::pair is a fixed-size collection of heterogeneous values.
  /// serialize/deserialize using std::apply-like method
  template <typename K, typename V>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API serialize_impl(std::pair<K, V> const& value)
  {
    co_await serialize(value.first);
    co_await serialize(value.second);
  }
  template <typename... Args>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API serialize_impl(std::tuple<Args ...> const& t)
  {
    if(not _M_range_dependent) COASYNC_ATTRIBUTE((likely))
      co_await _M_put_desc(_S_condense(std::type_index{ typeid(std::tuple<Args ...>) }.hash_code()));
    bool const previous_state = _M_range_dependent;
    _M_range_dependent = true;
    co_await _M_do_serialize_tuple(t);
    _M_range_dependent = previous_state;
  }
  /// To hack the source code, here is a simple way to register type information using meta<T>::field specialization
  template <typename T>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API serialize_impl(COASYNC_ATTRIBUTE((maybe_unused)) customized_tag_t, T const& value)
  {
    if(not _M_range_dependent) COASYNC_ATTRIBUTE((likely))
      co_await _M_put_desc(_S_condense(std::type_index{ typeid(T) }.hash_code()));
    bool const previous_state = _M_range_dependent;
    _M_range_dependent = true;
    co_await _M_do_serialize_field(value, meta<T>::fields);
    _M_range_dependent = previous_state;
  }
  template <typename T>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API serialize_impl(COASYNC_ATTRIBUTE((maybe_unused)) aggregate_tag_t, T const& t)
  {
    if(not _M_range_dependent) COASYNC_ATTRIBUTE((likely))
      co_await _M_put_desc(_S_condense(std::type_index{ typeid(T) }.hash_code()));
    bool const previous_state = _M_range_dependent;
    _M_range_dependent = true;
    co_await _M_do_serialize_aggregate(t);
    _M_range_dependent = previous_state;
  }
  template <typename T>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API deserialize_impl(COASYNC_ATTRIBUTE((maybe_unused)) smallint_tag_t, T& value)
  {
    std::make_unsigned_t<T> uvalue {};
    std::size_t offset {};
    COASYNC_ATTRIBUTE((gnu::uninitialized)) std::uint8_t b;
    do
      {
        co_await static_cast<crtp_object&>(*this)
        .read(reinterpret_cast<typename crtp_object::char_type*>(&b), sizeof(std::uint8_t));
        uvalue |= static_cast<std::make_unsigned_t<T>>(b & ~0x80) << offset;
        offset += 7;
      }
    while((b & 0x80) != 0ull);
    if constexpr(std::unsigned_integral<T>) value = uvalue;
    else value = static_cast<T>((uvalue >> 1) ^ - (uvalue & 1));
  }
  /// POD (Plain Old Data) is a concept in C++ that refers to a type of data that can be copied
  /// and transferred through simple memory replication. Pod-type objects can be copied by
  /// memcpy or other equivalent operations, and their memory layout is completely
  /// transparent and predictable.
  template <typename T>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API deserialize_impl(COASYNC_ATTRIBUTE((maybe_unused)) plain_tag_t, T& value)
  {
    if(not _M_range_dependent) COASYNC_ATTRIBUTE((likely))
      _S_deserial_verify<T>(co_await _M_get_desc());
    co_await static_cast<crtp_object&>(*this)
    .read(reinterpret_cast<typename crtp_object::char_type*>(&value), sizeof(T));
  }
  /// std::map/std::multimap/std::unorder_map/.etc value_type 	std::pair<const Key, T>
  /// The following type purification template is designed to remove constant qualifier of the key of std::pair
  template <typename T>
  struct pair_deduce
  {
    typedef T type;
  };
  template <typename T, typename U>
  struct pair_deduce<std::pair<const T, U>>
  {
    typedef std::pair<T, U> type;
  };
  template <typename T>
  using pair_deduce_t
    = typename pair_deduce<T>::type;
  /// Deserialization is almost the reverse of serialization
  template <typename Rng>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API deserialize_impl(COASYNC_ATTRIBUTE((maybe_unused)) range_tag_t, Rng& value)
  {
    if(not _M_range_dependent) COASYNC_ATTRIBUTE((likely))
      _S_deserial_verify<Rng>(co_await _M_get_desc());
    bool const previous_state = _M_range_dependent;
    _M_range_dependent = true;
    std::size_t size = co_await _M_get_desc();
    COASYNC_ATTRIBUTE((gnu::uninitialized)) pair_deduce_t<std::ranges::range_value_t<Rng>> element;
    COASYNC_ATTRIBUTE((gnu::uninitialized)) Rng swap_value;
    if constexpr(requires(Rng& rng)
    {
      rng.reserve(std::declval<std::size_t>());
      }) swap_value.reserve(size);
    for(std::size_t count {}; count < size; count ++)
      {
        co_await deserialize(element);
        if constexpr(
          requires(Rng& rng)
        {
          rng.push_back(std::declval<std::ranges::range_value_t<Rng>>());
          })
        swap_value.push_back(std::move(element));
        else if constexpr(
          requires(Rng& rng)
        {
          rng.push(std::declval<std::ranges::range_value_t<Rng>>());
          })
        swap_value.push(std::move(element));
      }
    static_assert(std::swappable<Rng>);
    std::ranges::swap(swap_value, value);
    _M_range_dependent = previous_state;
  }
  template <typename T>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API deserialize_impl(std::stack<T>& value)
  {
    if(not _M_range_dependent) COASYNC_ATTRIBUTE((likely))
      _S_deserial_verify<std::stack<T>>(co_await _M_get_desc());
    bool const previous_state = _M_range_dependent;
    _M_range_dependent = true;
    std::size_t size = co_await _M_get_desc();
    COASYNC_ATTRIBUTE((gnu::uninitialized)) T element;
    COASYNC_ATTRIBUTE((gnu::uninitialized)) std::stack<T> swap_value;
    for(std::size_t count {}; count < size; count ++)
      {
        co_await deserialize(element);
        swap_value.emplace(std::move(element));
      }
    value.swap(swap_value);
    _M_range_dependent = previous_state;
  }
  template <typename T>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API deserialize_impl(std::priority_queue<T>& value)
  {
    if(not _M_range_dependent) COASYNC_ATTRIBUTE((likely))
      _S_deserial_verify<std::priority_queue<T>>(co_await _M_get_desc());
    bool const previous_state = _M_range_dependent;
    _M_range_dependent = true;
    std::size_t size = co_await _M_get_desc();
    COASYNC_ATTRIBUTE((gnu::uninitialized)) T element;
    COASYNC_ATTRIBUTE((gnu::uninitialized)) std::priority_queue<T> swap_value;
    for(std::size_t count {}; count < size; count ++)
      {
        co_await deserialize(element);
        swap_value.emplace(std::move(element));
      }
    value.swap(swap_value);
    _M_range_dependent = previous_state;
  }
  template <typename T, typename Container>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API deserialize_impl(std::queue<T, Container>& value)
  {
    if(not _M_range_dependent) COASYNC_ATTRIBUTE((likely))
      _S_deserial_verify<std::queue<T, Container>>(co_await _M_get_desc());
    bool const previous_state = _M_range_dependent;
    _M_range_dependent = true;
    std::size_t size = co_await _M_get_desc();
    COASYNC_ATTRIBUTE((gnu::uninitialized)) T element;
    COASYNC_ATTRIBUTE((gnu::uninitialized)) std::queue<T, Container> swap_value;
    for(std::size_t count {}; count < size; count ++)
      {
        co_await deserialize(element);
        swap_value.emplace(std::move(element));
      }
    value.swap(swap_value);
    _M_range_dependent = previous_state;
  }
  template <typename K, typename V>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API deserialize_impl(std::pair<K, V>& value)
  {
    co_await deserialize(value.first);
    co_await deserialize(value.second);
  }
  template <typename... Args>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API deserialize_impl(std::tuple<Args...>& t)
  {
    if(not _M_range_dependent) COASYNC_ATTRIBUTE((likely))
      _S_deserial_verify<std::tuple<Args...>>(co_await _M_get_desc());
    bool const previous_state = _M_range_dependent;
    _M_range_dependent = true;
    co_await _M_do_deserialize_tuple(t);
    _M_range_dependent = previous_state;
  }
  template <typename T>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API deserialize_impl(COASYNC_ATTRIBUTE((maybe_unused)) customized_tag_t, T& value)
  {
    if(not _M_range_dependent) COASYNC_ATTRIBUTE((likely))
      _S_deserial_verify<T>(co_await _M_get_desc());
    bool const previous_state = _M_range_dependent;
    _M_range_dependent = true;
    co_await _M_do_deserialize_field(value, meta<T>::fields);
    _M_range_dependent = previous_state;
  }
  template <typename T>
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> COASYNC_API deserialize_impl(COASYNC_ATTRIBUTE((maybe_unused)) aggregate_tag_t, T& t)
  {
    if(not _M_range_dependent) COASYNC_ATTRIBUTE((likely))
      _S_deserial_verify<T>(co_await _M_get_desc());
    bool const previous_state = _M_range_dependent;
    _M_range_dependent = true;
    co_await _M_do_deserialize_aggregate(t);
    _M_range_dependent = previous_state;
  }
protected:
  template <typename T>
  COASYNC_ATTRIBUTE((always_inline))
  static void _S_deserial_verify(desc_t c)
  {
    if(_S_condense(std::type_index{typeid(T)}.hash_code()) == c) COASYNC_ATTRIBUTE((likely))
      return;
    throw std::runtime_error {typeid(T).name()};
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  static constexpr desc_t _S_condense(unsigned long long u64) noexcept
  {
    unsigned int u32 = static_cast<unsigned int>(u64 >> 32) ^ static_cast<unsigned int>(u64);
    desc_t u16 = static_cast<desc_t>(u32 >> 16) ^ static_cast<desc_t>(u32);
    return (u16);
  }
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<void> _M_put_desc(desc_t tag)
  {
    co_await static_cast<crtp_object&>(*this)
    .write(reinterpret_cast<typename crtp_object::char_type const*>(&tag), sizeof(desc_t));
  }
  COASYNC_ATTRIBUTE((nodiscard))
  awaitable<desc_t> _M_get_desc()
  {
    COASYNC_ATTRIBUTE((gnu::uninitialized)) desc_t tag;
    co_await static_cast<crtp_object&>(*this)
    .read(reinterpret_cast<typename crtp_object::char_type*>(&tag), sizeof(desc_t));
    co_return (tag);
  }
  bool _M_range_dependent = false;
};
}
}
#endif
