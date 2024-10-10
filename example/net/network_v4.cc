#include "../../include/coasync/net/network_v4.hpp"
#include <iostream>
using namespace coasync;
int main()
{
  net::network_v4 nw(net::make_address_v4("192.0.2.0"), 24);

  std::cout << "Network address: " << nw.network() << std::endl;
  std::cout << "Broadcast address: " << nw.broadcast() << std::endl;

  std::cout << "Network address mask: " << nw.netmask() << std::endl;

  std::cout << "Prefix length: " << nw.prefix_length() << std::endl;

	for(auto addr: nw.hosts())
		std::cout << "network host: " << addr << std::endl;

}
