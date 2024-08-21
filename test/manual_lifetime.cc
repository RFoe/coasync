#include "../include/coasync/detail/manual_lifetime.hpp"
#include <cstdio>
using namespace coasync::detail;

struct X {
	X() {
		std::puts("X()");
	}
	~X() {
		std::puts("~X()");
	}

};

struct Y {
	manual_lifetime<X> _M_value;
};

void foo(manual_lifetime<X>& value) {
 value.construct();
 value.destruct();
}
int main() {
	[[gnu::uninitialized]] manual_lifetime<X> manual_lifetime;
	foo(manual_lifetime);
}
