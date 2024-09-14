#ifndef COASYNC_SERVICE_REGISTRY_INCLUDED
#define COASYNC_SERVICE_REGISTRY_INCLUDED

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "config.hpp"
#include <memory>
#if defined(__cpp_lib_memory_resource)
# include <memory_resource>
#else
# error This library requires the use of C++17 pmr support
#endif

namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) coasync
{
struct execution_context; /// forward declaration
namespace COASYNC_ATTRIBUTE((gnu::visibility("default"))) detail
{
struct null_service_t
{
  explicit null_service_t(auto&) {}
  void post_frame(auto&& ...) {}
  void commit_frame(auto&& ...) {}
};
/// empty tag type
/// The null type acts as the virtual head node of the intrusive linked list
template <typename execution_context>
struct basic_service_registry
{
private:
  inline static std::atomic_long S_type_count = 0;
  template <typename T>
  struct type_id
  {
    inline static long const S_value = S_type_count.fetch_add(1, std::memory_order_relaxed);
  };
  enum Op
  {
    Op_new,
    Op_delete,
    Op_commit,
    Op_wait,
  };
  template <typename service_type>
  struct manager
  {
    static void S_manage(Op opcode, std::atomic<void*>* service, execution_context* context)
    {
      if constexpr(std::is_same_v<service_type, null_service_t>) return;
      switch(opcode)
        {
        case Op_new:
				  service -> store(std::pmr::polymorphic_allocator<std::byte> { context -> memory_resource() }.new_object<service_type>(*context), std::memory_order_release);
          service -> notify_all();
          break;
        case Op_delete:
          if(not service -> load(std::memory_order_acquire)) service -> wait(nullptr);
          std::pmr::polymorphic_allocator<std::byte> { context -> memory_resource() }
          .delete_object(static_cast<service_type*>(service -> load(std::memory_order_acquire)));
          break;
        case Op_commit:
          if(not service -> load(std::memory_order_acquire)) service -> wait(nullptr);
          static_cast<service_type*>(service -> load(std::memory_order_acquire)) -> commit_frame();
          break;
        case Op_wait:
          if(not service -> load(std::memory_order_acquire)) service -> wait(nullptr);
          break;
        }
    }
  };
public:
  template <typename service_type>
  basic_service_registry(COASYNC_ATTRIBUTE((maybe_unused)) std::in_place_type_t<service_type>) noexcept
    : _M_service(nullptr)
    , _M_type_id(type_id<service_type>::S_value)
    , _M_next(nullptr)
    , _M_manage(&manager<service_type>::S_manage)
  {
  }

	basic_service_registry& operator=(basic_service_registry const&) = delete;
  
  friend void erase_after(basic_service_registry* position, execution_context& context)
  {
    position = position -> _M_next.load(std::memory_order_acquire);
    while (position != nullptr)
      {
        basic_service_registry* tmp = (position -> _M_next).load(std::memory_order_acquire);
        (* position -> _M_manage )(Op_delete, &position -> _M_service, &context);
        delete position;
        position = tmp;
      }
  }
  friend void commit_after(basic_service_registry* position, execution_context& context)
  {
    position = position -> _M_next.load(std::memory_order_acquire);
    while(position != nullptr)
      {
        (* position -> _M_manage )(Op_commit, &position -> _M_service, &context);
        position = position -> _M_next.load(std::memory_order_acquire);
      }
  }
  template <typename service_type>
  COASYNC_ATTRIBUTE((nodiscard)) friend service_type& find_from(basic_service_registry* position, execution_context& context)
  {
    std::unique_ptr<basic_service_registry> prepare =
      std::make_unique<basic_service_registry>(std::in_place_type<service_type>);
    long service_id = type_id<service_type>::S_value;
    while (true)
      {
        if (position -> _M_type_id == service_id) COASYNC_ATTRIBUTE((unlikely))
          {
            (* position -> _M_manage )(Op_wait, &position -> _M_service, &context);
            return * static_cast<service_type*>(position -> _M_service.load(std::memory_order_acquire));
          }
        if (position -> _M_next.load(std::memory_order_acquire) == nullptr)
          {
            basic_service_registry* old_nullptr = nullptr;
            if (position -> _M_next.compare_exchange_weak(old_nullptr, prepare.get(),
                std::memory_order_release, std::memory_order_relaxed)) COASYNC_ATTRIBUTE((likely))
              {
                prepare.release();
                basic_service_registry* target = position -> _M_next.load(std::memory_order_acquire);
                (* target -> _M_manage )(Op_new, &target -> _M_service, &context);
                return * static_cast<service_type*>(target -> _M_service.load(std::memory_order_acquire));
              }
          }
        position = position -> _M_next.load(std::memory_order_acquire);
      }
  }
private:
  std::atomic<void*> 										_M_service;
  long const 														_M_type_id;
  std::atomic<basic_service_registry*>  _M_next;
  void(*_M_manage)(Op, std::atomic<void*>*, execution_context*);
};
typedef basic_service_registry<execution_context> service_registry;
}
}
#endif
