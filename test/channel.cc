#include "../include/coasync/execution_context.hpp"
#include "../include/coasync/functional.hpp"
#include "../include/coasync/channel.hpp"
#include "../include/coasync/co_spawn.hpp"
using namespace coasync;
using std::chrono::operator""s;
awaitable<void> do_send(channel<int, 8>& channel) {
	for(unsigned int count {}; count < 8; count ++) {
		co_await sleep_for(1s);
		co_await channel.send(std::rand());
	}
}
awaitable<void> do_receive(channel<int, 8>& channel) {
	for(unsigned int count {}; count < 8; count ++)
		std::printf("receive: %d\n", co_await channel.receive());
}
int main() {
 execution_context context{2};
 channel<int, 8> channel;
 co_spawn(context, do_send(channel), use_detach);
 co_spawn(context, do_send(channel), use_detach);
 co_spawn(context, do_send(channel), use_detach);
 co_spawn(context, do_receive(channel), use_detach);
 co_spawn(context, do_receive(channel), use_detach);
 co_spawn(context, do_receive(channel), use_detach);
 context.loop();
}
