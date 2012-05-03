#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include <arpa/inet.h>

#include <SocketAddress.hpp>

using namespace std;

SocketAddress::SocketAddress(const sockaddr* addr)
{
	memset(&this->addr_, 0, sizeof(this->addr_));

	switch (addr->sa_family) {
		case AF_INET:
			this->addr_.in = *reinterpret_cast<const sockaddr_in*>(addr);
			break;

		case AF_INET6:
			this->addr_.in6 = *reinterpret_cast<const sockaddr_in6*>(addr);
			break;

		default:
			throw invalid_argument("Unsupported address family");
	}
}

SocketAddress::SocketAddress(IPAddress&& addr, in_port_t port)
{
	memset(&addr_, 0, sizeof(addr_));

	addr_.in.sin_family = addr.native_family();
	addr_.in.sin_port = htons(port);
	switch (addr.family()) {
		case AddressFamily::IPv4:
			addr_.in.sin_addr = addr.in();
			break;

		case AddressFamily::IPv6:
			addr_.in6.sin6_addr = addr.in6();
			break;
	}
}
