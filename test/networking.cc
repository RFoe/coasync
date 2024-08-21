#include "../include/coasync/detail/networking.hpp"
using namespace coasync;
int main() {
	WSADATA wsa_data;
	::WSAStartup(MAKEWORD(2, 2), std::addressof(wsa_data));
	std::printf("%s\n", ::gai_strerror(EAI_NONAME));
	throw std::system_error(EAI_NONAME, detail::generic_category());
 	WSACleanup();
}
