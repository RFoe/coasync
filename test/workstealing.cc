#include "../include/coasync/awaitable.hpp"
#include "../include/coasync/detail/workstealing.hpp"
using namespace coasync;
using namespace coasync::detail;
awaitable<void> test_nested() noexcept {
	std::puts("test_nested");
	co_return;
}
awaitable<void> test() noexcept {
	std::puts("test");
	co_await test_nested();
	co_return;
}

int main() {
	workstealing s;
//	s.push(test().release_coroutine());
	auto* p = s.try_pop();
	std::coroutine_handle<>::from_address(p).resume();
}
