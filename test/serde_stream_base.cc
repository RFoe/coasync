#include "../include/coasync/detail/meta/serde_stream_base.hpp"
#include <sstream>
using namespace coasync;
using namespace coasync::detail;
struct serde_stream: serde_stream_base<serde_stream>, public std::stringstream
{
  virtual ~ serde_stream() {}
	using serde_stream_base<serde_stream>::operator>>;
	using serde_stream_base<serde_stream>::operator<<;
};
void test() {
	  serde_stream s;
	  std::stack<int> V {{1, 2}};
		s << V << 33llu;
	  std::stack<int> VV;
	 unsigned long long LL;
	  s >> VV >> LL;
	  while(not VV.empty()) {
			std::printf("%u\n", VV.top());
			VV.pop();
		}
		std::printf("%llu\n", LL);
}
int main()
{
	test();
}
