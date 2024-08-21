#include "../include/coasync/detail/config.h"

#include <cstdio>

#define coasync_to_string(__x) #__x

[[nodiscard]] coasync_attribute(always_inline) int COASYNC_API hello() {
	std::puts("hello world");
	return 8;
}

int main() {
 (void)hello();
}
