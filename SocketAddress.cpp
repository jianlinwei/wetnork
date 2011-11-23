#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "network.hpp"

SocketAddress::SocketAddress()
{
}

int SocketAddress::family() const
{
	return addr.in.sin_family;
}

const sockaddr* SocketAddress::native() const
{
	return reinterpret_cast<const sockaddr*>(&addr);
}

socklen_t SocketAddress::native_len() const
{
	return len;
}

SocketAddress::SocketAddress(const sockaddr* addr)
{
	const sockaddr_in* sin = reinterpret_cast<const sockaddr_in*>(addr);

	switch (sin->sin_family) {
		case AF_INET:
			len = sizeof(sockaddr_in);
			break;

		case AF_INET6:
			len = sizeof(sockaddr_in6);
			break;

		default:
			throw BadAddress("Unsupported address family");
	}

	memcpy(&this->addr, addr, len);
}

SocketAddress SocketAddress::parse(const std::string& addr, in_port_t port)
{
	SocketAddress result;

	memset(&result, 0, sizeof(result));

	if (addr.find(':') < addr.size()) {
		result.len = sizeof(sockaddr_in6);
		if (inet_pton(AF_INET6, addr.c_str(), &result.addr.in6.sin6_addr) < 0) {
			throw BadAddress("Cannot parse IP address");
		}
		result.addr.in6.sin6_port = htons(port);
	} else {
		result.len = sizeof(sockaddr_in);
		if (inet_pton(AF_INET, addr.c_str(), &result.addr.in.sin_addr) < 0) {
			throw BadAddress("Cannot parse IP address");
		}
		result.addr.in.sin_port = htons(port);
	}

	return result;
}

bool SocketAddress::operator<(const SocketAddress& other) const
{
	return len < other.len
		? -1
		: memcmp(&addr, &other.addr, len);
}

bool SocketAddress::operator==(const SocketAddress& other) const
{
	return len == other.len
		&& memcmp(&addr, &other.addr, len) == 0;
}

