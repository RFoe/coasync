#ifndef COASYNC_RESOLVER_INCLUDED
#define COASYNC_RESOLVER_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if defined(__has_include)
#	if defined(__linux__) && __has_include(<netdb.h>)
#   #include<netdb.h> 							/// for ::getaddrinfo
#	endif
#endif
#include "../detail/networking.hpp" /// for detail::generic_category, ::getaddrinfo
#include "../co_spawn.hpp"          /// for co_spawn
#include "resolver_flags.hpp"       /// for resolver_flags

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) net
{
enum class resolver_errc : std::underlying_type_t<std::errc>
{
  host_not_found = EAI_NONAME,
  host_not_found_try_again = EAI_AGAIN,
  service_not_found = EAI_SERVICE,
  // N.B. POSIX defines additional errors that have no enumerator here:
  // EAI_BADFLAGS, EAI_FAIL, EAI_FAMILY, EAI_MEMORY, EAI_SOCKTYPE, EAI_SYSTEM
  // Some C libraries define additional errors:
  // EAI_BADHINTS, EAI_OVERFLOW, EAI_PROTOCOL
  // Some C libraries define additional (obsolete?) errors:
  // EAI_ADDRFAMILY, EAI_NODATA
};
COASYNC_ATTRIBUTE((nodiscard, always_inline))
std::error_category const& COASYNC_API resolver_category() noexcept
{
/// specified error category to build an system_error
#if defined(__linux__)
  struct category_t : std::error_category
  {
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    const char* name() const noexcept
    {
      return "resolver";
    }
    COASYNC_ATTRIBUTE((nodiscard, always_inline))
    std::string message(int __e) const
    {
      /// ::gai_strerror in WINDOWS is not surported
      return ::gai_strerror(__e);
    }
  };
  static category_t category;
  return category;
#elif defined(_WIN32) || defined(_WIN64)
  /// to print out errno returned by ::getaddrinfo/::getaddrname in WINDOWS,
  /// need to use ::WSAFormatError in detail::generic_cateogry
  /**
   * Use the gai_strerror function to print error messages based on the EAI codes
   * returned by the getaddrinfo function. The gai_strerror function is provided
   * for compliance with IETF recommendations, but it is not thread safe. Therefore,
   * use of traditional Windows Sockets functions such as WSAGetLastError is recommended.
   **/
  return detail::generic_category();
#endif
}
COASYNC_ATTRIBUTE((nodiscard, always_inline))
std::error_code COASYNC_API make_error_code(resolver_errc __e) noexcept
{
  return std::error_code(static_cast<int>(__e), resolver_category());
}
COASYNC_ATTRIBUTE((nodiscard, always_inline))
std::error_condition COASYNC_API make_error_condition(resolver_errc __e) noexcept
{
  return std::error_condition(static_cast<int>(__e), resolver_category());
}
template <typename Protocol>
struct basic_resolver_entry
{
  typedef Protocol 											procotol_type;
  typedef typename Protocol::endpoint 	endpoint_type;
  COASYNC_ATTRIBUTE((always_inline))
  explicit basic_resolver_entry() { }
  template <typename StrHost, typename StrService>
  COASYNC_ATTRIBUTE((always_inline))
  basic_resolver_entry(
    endpoint_type const& 	endpoint,
    StrHost&& 						host,
    StrService&& 					service
    /// Keep moving semantics as much as possible to reduce copy overhead
    /// char const*, std::string, std::string_view, .etc
  )
    : _M_endpoint(endpoint)
    , _M_host(		std::forward<StrHost&&>(host))
    , _M_service(	std::forward<StrService&&>(service))
  { }
  /// The endpoint type holds the address and port fields, which is light
  /// in memory usage and may not be necessary to provide a reference access interface
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  endpoint_type COASYNC_API endpoint() const
  {
    return _M_endpoint;
  }
  /// The endpoint type holds the address and port fields, which is light
  /// in memory usage and may not be necessary to provide a reference access interface
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  COASYNC_API operator endpoint_type() const
  {
    return _M_endpoint;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  std::string COASYNC_API host_name() const
  {
    return _M_host;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  std::string COASYNC_API service_name() const
  {
    return _M_service;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend bool COASYNC_API operator==(basic_resolver_entry const& a, basic_resolver_entry const& b) noexcept
  {
    return a._M_endpoint == b._M_endpoint
           and a._M_host == b._M_host
           and a._M_service == b._M_service;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend bool COASYNC_API operator!=(basic_resolver_entry const& a, basic_resolver_entry const& b) noexcept
  {
    return !(a == b);
  }
private:
  endpoint_type _M_endpoint;
  std::string 	_M_host;
  std::string 	_M_service;
};
template <typename Protocol>
struct basic_resolver_results: std::ranges::view_interface<basic_resolver_results<Protocol>>
{
  typedef Protocol 														protocol_type;
  typedef typename protocol_type::endpoint 		endpoint_type;
  typedef basic_resolver_entry<protocol_type> value_type;
  typedef const value_type& 									const_reference;
  typedef value_type& 												reference;
  /// adapter type of std::forward_list
  /// has iterable[for range-based loop] and immutable traits
  typedef typename std::forward_list<value_type>::const_iterator
  const_iterator;
  typedef const_iterator 											iterator;
  typedef std::ptrdiff_t 											difference_type;
  typedef std::size_t 												size_type;
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  size_type COASYNC_API size() const noexcept
  {
    return _M_size;
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  size_type COASYNC_API max_size() const noexcept
  {
    return _M_results.max_size();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  bool COASYNC_API empty() const noexcept
  {
    return _M_results.empty();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  const_iterator COASYNC_API begin() const
  {
    return _M_results.begin();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  const_iterator COASYNC_API end() const
  {
    return _M_results.end();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  const_iterator COASYNC_API cbegin() const
  {
    return _M_results.begin();
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  const_iterator COASYNC_API cend() const
  {
    return _M_results.end();
  }
  COASYNC_ATTRIBUTE((always_inline))
  void COASYNC_API swap(basic_resolver_results& other) noexcept
  {
    _M_results.swap(other._M_results);
    std::swap(_M_size, other._M_size);
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend bool COASYNC_API operator==(basic_resolver_results const& a, basic_resolver_results const& b) noexcept
  {
    return a._M_size == b._M_size
           /// std::equal is forwarding algorithm
           and std::equal(a._M_results.begin(), a._M_results.end(), b._M_results.begin());
  }
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  friend bool COASYNC_API operator!=(basic_resolver_results const& a, basic_resolver_results const& b) noexcept
  {
    return !(a == b);
  }
private:
  template <typename OtherProtocol, typename execution_context>
  friend struct __basic_resolver;
  COASYNC_ATTRIBUTE((always_inline))
  basic_resolver_results(std::string_view, std::string_view, resolver_flags, protocol_type* = nullptr);
  COASYNC_ATTRIBUTE((always_inline))
  basic_resolver_results(endpoint_type const&);
  std::forward_list<value_type> _M_results;
  /// std::forward list does not maintain size and requires additional fields to
  /// track the length of the linked list
  std::size_t 									_M_size = 0;
};
template <typename Protocol, typename execution_context>
struct __basic_resolver
{
	///Asynchronous domain name resolution is achieved by submitting concurrent tasks with co spawn,
	/// which is blocked synchronously if the associated scheduler is single-threaded
  typedef execution_context 								context_type;
  typedef Protocol 													protocol_type;
  typedef Protocol::endpoint 								endpoint_type;
  typedef basic_resolver_results<Protocol> 	results_type;
  COASYNC_ATTRIBUTE((nodiscard, always_inline))
  execution_context& context() noexcept
  {
    return _M_context;
  }
  template <typename CompletionToken>
  COASYNC_ATTRIBUTE((nodiscard))
  auto COASYNC_API resolve(
    protocol_type 			 	protocol,
    std::string 					host,
    std::string 					service,
    /// Function calls may be asynchronous and need to move the lifecycle inside
    /// the coroutine, so value passing is used
    COASYNC_ATTRIBUTE((maybe_unused)) CompletionToken
/// use_future, use_awaitable, use_detach
  )
  {
    return resolve(std::move(protocol), std::move(host), std::move(service), resolver_flags(0), CompletionToken());
  }
  template <typename CompletionToken>
  COASYNC_ATTRIBUTE((nodiscard))
  auto COASYNC_API resolve(
    std::string 					host,
    std::string 					service,
    /// Function calls may be asynchronous and need to move the lifecycle inside
    /// the coroutine, so value passing is used
    COASYNC_ATTRIBUTE((maybe_unused)) CompletionToken
    /// use_future, use_awaitable, use_detach
  )
  {
    return resolve(std::move(host), std::move(service), resolver_flags(0), CompletionToken());
  }
  template <typename CompletionToken>
  COASYNC_ATTRIBUTE((nodiscard))
  auto COASYNC_API resolve(
    protocol_type 			 	protocol,
    std::string 					host,
    std::string 					service,
    /// Function calls may be asynchronous and need to move the lifecycle inside
    /// the coroutine, so value passing is used
    resolver_flags 				flags,
    COASYNC_ATTRIBUTE((maybe_unused)) CompletionToken
    /// use_future, use_awaitable, use_detach
  )
  {
    return co_spawn(_M_context,
                    [](
                      protocol_type 				protocol,
                      std::string 					host,
                      std::string 					service,
                      resolver_flags 				flags
                    ) -> awaitable<results_type>
    {
      co_return results_type { host, service, flags, &protocol };
    }(std::move(protocol), std::move(host), std::move(service), flags), CompletionToken());
  }
  template <typename CompletionToken>
  COASYNC_ATTRIBUTE((nodiscard))
  auto COASYNC_API resolve(
    std::string 					host,
    std::string 					service,
    /// Function calls may be asynchronous and need to move the lifecycle inside
    /// the coroutine, so value passing is used
    resolver_flags 				flags,
    COASYNC_ATTRIBUTE((maybe_unused)) CompletionToken
    /// use_future, use_awaitable, use_detach
  )
  {
    return co_spawn(_M_context,
                    [](
                      std::string 					host,
                      std::string 					service,
                      resolver_flags 				flags
                    ) -> awaitable<results_type>
    {
      co_return results_type { host, service, flags, nullptr };
    }(std::move(host), std::move(service), flags), CompletionToken());
  }
  template <typename CompletionToken>
  COASYNC_ATTRIBUTE((nodiscard))
  auto COASYNC_API resolve(
    endpoint_type endpoint,
    /// Function calls may be asynchronous and need to move the lifecycle inside
    /// the coroutine, so value passing is used
    COASYNC_ATTRIBUTE((maybe_unused)) CompletionToken
    /// use_future, use_awaitable, use_detach
  )
  {
    return co_spawn(_M_context,
                    [](
                      endpoint_type endpoint
                    ) -> awaitable<results_type>
    {
      co_return results_type { endpoint };
    }(std::move(endpoint)), CompletionToken());
  }
  COASYNC_ATTRIBUTE((always_inline))
  __basic_resolver(execution_context& context) noexcept
    : _M_context(context)
  {}
  COASYNC_ATTRIBUTE((always_inline))
  __basic_resolver(__basic_resolver const&) = default;
  COASYNC_ATTRIBUTE((always_inline))
  __basic_resolver& operator=(__basic_resolver const&) = default;
  COASYNC_ATTRIBUTE((always_inline))
  __basic_resolver(__basic_resolver&&) = default;
  COASYNC_ATTRIBUTE((always_inline))
  __basic_resolver& operator=(__basic_resolver&&) = default;
  COASYNC_ATTRIBUTE((always_inline))
  ~ __basic_resolver() = default;
private:
  execution_context& _M_context;
};
template <typename Protocol>
using basic_resolver = __basic_resolver<execution_context, Protocol>;
template<typename Protocol>
basic_resolver_results<Protocol>::basic_resolver_results(
  std::string_view 		host_name,
  std::string_view 		service_name,
  resolver_flags 			flags,
  protocol_type* 			protocol
)
{
  std::string host;
  const char* host_view = not host_name.empty()
                          ? (host.assign(host_name.data(), host_name.size())).c_str()
                          : nullptr;
  std::string service;
  const char* service_view = not service_name.empty()
                             ? (service.assign(host_name.data(), host_name.size())).c_str()
                             : nullptr;
  ::addrinfo hints{ };
  hints.ai_flags = static_cast<int>(flags);
  if (protocol != nullptr)
    {
      hints.ai_family = static_cast<int>(protocol->family());
      hints.ai_socktype = static_cast<int>(protocol->category());
      hints.ai_protocol = static_cast<int>(protocol->protocol());
    }
  else
    {
      auto local_protocol = endpoint_type{}.protocol();
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = static_cast<int>(local_protocol.category());
      hints.ai_protocol = static_cast<int>(local_protocol.protocol());
    }
  struct scoped_addrinfo
  {
    ~scoped_addrinfo()
    {
      if (_M_p) ::freeaddrinfo(_M_p);
    }
    ::addrinfo* _M_p = nullptr;
  } sai;
  /// the ANSI version of a function that provides protocol-independent translation
	/// from host name to address.
	/// The getaddrinfo function returns results for the NS_DNS namespace. The getaddrinfo
	/// function aggregates all responses if more than one namespace provider returns
	/// information. For use with the IPv6 and IPv4 protocol, name resolution can be
	/// by the Domain Name System (DNS), a local hosts file, or by other naming mechanisms
	/// for the NS_DNS namespace.The getaddrinfo function returns results for the
	/// NS_DNS namespace. The getaddrinfo function aggregates all responses if more
	/// than one namespace provider returns information. For use with the IPv6 and
	/// IPv4 protocol, name resolution can be by the Domain Name System (DNS), a local
	/// hosts file, or by other naming mechanisms for the NS_DNS namespace.
  if (int __err = ::getaddrinfo(host_view, service_view, &hints, &sai._M_p))
    throw std::system_error(__err, resolver_category());
  auto tail = _M_results.before_begin();
  for (auto ai_entry = sai._M_p; ai_entry != nullptr; ai_entry = ai_entry->ai_next)
    {
      if (ai_entry->ai_family == AF_INET)
        {
          tail = _M_results.emplace_after(
                   tail,
                   endpoint_type::from_sockaddr_in(*reinterpret_cast<::sockaddr_in*>(ai_entry->ai_addr)),
                   host, service);
        }
      else if (ai_entry->ai_family == AF_INET6)
        {
          tail = _M_results.emplace_after(
                   tail,
                   endpoint_type::from_sockaddr_in6(*reinterpret_cast<::sockaddr_in6*>(ai_entry->ai_addr)),
                   host, service);
        }
      else continue;
      _M_size++;
    }
}
template<typename _InternetProtocol>
basic_resolver_results<_InternetProtocol>::basic_resolver_results(endpoint_type const& endpoint)
{
  char 	host_name[1025];	// glibc NI_MAXHOST
  char 	service_name[32];  // glibc NI_MAXSERV
  int 	flags = 0;
  if (static_cast<int>(endpoint.protocol().category()) == SOCK_DGRAM) flags |= NI_DGRAM;
  ::sockaddr_in 	__sockaddr_in;
  ::sockaddr_in6 	__sockaddr_in6;;
  if(endpoint.protocol() == protocol_type::v4())
    __sockaddr_in = endpoint.to_sockaddr_in();
  else
    __sockaddr_in6 = endpoint.to_sockaddr_in6();
  const ::sockaddr* __sockaddr = endpoint.protocol() == protocol_type::v4()
                                 ? reinterpret_cast<const ::sockaddr*>(&__sockaddr_in)
                                 : reinterpret_cast<const ::sockaddr*>(&__sockaddr_in6);
  ::socklen_t __addrlen = endpoint.protocol() == protocol_type::v4()
                          ? sizeof(::sockaddr_in)
                          : sizeof(::sockaddr_in6);
  /// ANSI version of a function that provides protocol-independent name resolution. The getnameinfo
	/// function is used to translate the contents of a socket address structure to a node name and/or
	/// a service name.
	/// For IPv6 and IPv4 protocols, Name resolution can be by the Domain Name System (DNS), a local
	/// hosts file, or by other naming mechanisms. This function can be used to determine the host
	/// dname for an IPv4 or IPv6 address, a reverse DNS lookup, or determine the service name for a
	/// port number. The getnameinfo function can also be used to convert an IP address or a port
	/// number in a sockaddr structure to an ANSI string. This function can also be used to determine
	/// the IP address for a host name.
  int __err = ::getnameinfo(__sockaddr, __addrlen,
                            host_name, sizeof(host_name),
                            service_name, sizeof(service_name),
                            flags);
  if (__err)
    {
      flags |= NI_NUMERICSERV;
      __err = ::getnameinfo(__sockaddr, __addrlen,
                            host_name, sizeof(host_name),
                            service_name, sizeof(service_name),
                            flags);
    }
  if(__err)
    throw std::system_error(__err, resolver_category());
  else
    {
      _M_results.emplace_front(endpoint, host_name, service_name);
      _M_size = 1;
    }
}
}
}
#endif
