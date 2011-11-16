#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "network.hpp"

SocketAddress::SocketAddress(sockaddr* addr)
{
	sockaddr_in* sin = reinterpret_cast<sockaddr_in*>(addr);

	switch (sin->sin_family) {
		case AF_INET:
			len = sizeof(sockaddr_in);
			break;

		case AF_INET6:
			len = sizeof(sockaddr_in6);
			break;

		default:
			throw bad_address();
	}

	memcpy(&this->addr, addr, len);
}

SocketAddress SocketAddress::parse(std::string addr, in_port_t port)
{
	SocketAddress result;

	memset(&result, 0, sizeof(result));

	if (addr.find(':') < addr.size()) {
		result.len = sizeof(sockaddr_in6);
		if (inet_pton(AF_INET6, addr.c_str(), &result.addr.in6.sin6_addr) < 0) {
			throw bad_address();
		}
		result.addr.in6.sin6_port = htons(port);
	} else {
		result.len = sizeof(sockaddr_in);
		if (inet_pton(AF_INET, addr.c_str(), &result.addr.in.sin_addr) < 0) {
			throw bad_address();
		}
		result.addr.in.sin_port = htons(port);
	}

	return result;
}

