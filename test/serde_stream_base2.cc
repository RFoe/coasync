#include "../include/coasync/detail/meta/serde_stream_base.hpp"
#include <sstream>
#include <iostream>

using namespace coasync;
using namespace coasync::detail;
struct serde_stream: serde_stream_base<serde_stream>, public std::stringstream
{
  virtual ~ serde_stream() {}
	using serde_stream_base<serde_stream>::serialize;
	using serde_stream_base<serde_stream>::deserialize;
	awaitable<void> read( char_type* s, std::streamsize count) {
		std::stringstream::read(s, count);
		co_return;
	}
	awaitable<void> write( char_type const* s, std::streamsize count) {
		std::stringstream::write(s, count);
		co_return;
	}
};
awaitable<void> test() {
	  serde_stream s;
	  std::stack<int> V {{1, 2}};
	  co_await s.serialize(V);
		co_await s.serialize(12llu);
	  std::stack<int> VV;
	 unsigned long long LL;
	 co_await s.deserialize(VV);
	 co_await s.deserialize(LL);
	  while(not VV.empty()) {
			std::printf("%u\n", VV.top());
			VV.pop();
		}
		std::printf("%llu\n", LL);
		std::tuple<int, long, char, double> T { 2, 3llu, 's', 333.33};
		co_await s.serialize(T);
		std::tuple<int, long, char, double> TT;
		co_await s.deserialize(TT);
		auto [a, b, c, d] = TT;
		std::cout << a << " " << b << " " << (char)c << " " << d << "\n";
}
int main()
{
	test().get_coroutine()();
}
