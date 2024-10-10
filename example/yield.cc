#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/functional.hpp"
#include "../include/coasync/co_spawn.hpp"
using namespace coasync;
using std::chrono::operator""s;
awaitable<void> A() noexcept {
	for(unsigned int i {}; i < 5; i ++) {
	std::this_thread::sleep_for(1s);
		std::puts("-----------A------------");
		co_await yield();
	}
}
awaitable<void> B() noexcept {
	for(unsigned int i {}; i < 5; i ++) {
	std::this_thread::sleep_for(1s);
		std::puts("-----------B------------");
		co_await yield();
	}
}

int main() {
	execution_context context{concurrency_arg(0)};
	co_spawn(context, A(), use_detach);
	co_spawn(context, B(), use_detach);
	context.loop();
}
