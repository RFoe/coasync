#include "../include/coasync/awaitable.hpp"
#include <iostream>

using namespace coasync;

template <typename> struct function;
template <typename R, typename... Args>
struct function<awaitable<R>(Args ...)> {
	struct base {
	virtual awaitable<R> call(Args ...) = 0;
	};
	template <typename F>
	struct impl: base{
		impl(F&& f): _M_f((F&&) f) {}
		F _M_f;
	virtual awaitable<R> call(Args ...args) override {
		std::puts("call");
		return _M_f((Args) args ...);
	}
	};
	awaitable<R> operator()(Args ...args) {
		std::puts("operator()");
		 return _M_ptr -> call((Args) args ...);
	};
	template <typename F>
	function(F&& f) : _M_ptr(std::make_unique<impl<F>>(((F &&)f))) {}
	std::unique_ptr<base> _M_ptr;
};

awaitable<void> async_main() {
	function<awaitable<int>(int)> f { [value = 8](int x) mutable -> awaitable<int> { co_return x + value ++; } };
	std::printf("%d\n", co_await f(1));
	std::printf("%d\n", co_await f(2));
	std::printf("%d\n", co_await f(3));
}
int main() {
 async_main().get_coroutine()();
}
