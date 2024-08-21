#include "../include/coasync/awaitable.hpp"
#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/co_spawn.hpp"
#include "../include/coasync/detail/meta/function_traits.hpp"
#include <any>
using namespace coasync;

template <typename> struct co_function;
template <typename R, typename... Args> struct co_function<awaitable<R>(Args ...)> {
 template <typename F>
 struct delegate {
	 static awaitable<R> _S_fp(std::any* __any, Args ...args) {
		 return (* std::any_cast<F>(__any))((Args) args...);
	 }
 };
 co_function() {}
 template <typename F>
 co_function(F&& __f): _M_f((F&&)__f), _M_call(&delegate<F>::_S_fp) {}
 awaitable<R> operator()(Args... args) {
	 return (* _M_call)(&_M_f, (Args) args...);
 }
 std::any _M_f;
 awaitable<R>(* _M_call)(std::any*, Args ...);
};
template <typename F> co_function(F&&) -> co_function<typename detail::function_traits<F>::function>;
awaitable<void> async_main() {
	co_function f {[value = 8](int x) mutable -> awaitable<int> { co_return x + value ++; }};
	std::printf("%d\n", co_await f(8));
	std::printf("%d\n", co_await f(3));
	std::printf("%d\n", co_await f(9));
}
int main() {
 execution_context context;
 co_spawn(context, async_main(), use_detach); context.loop();
}
