#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include <arpa/inet.h>

#include <network.hpp>

using namespace std;

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
	memset(&this->addr, 0, sizeof(this->addr));

	switch (addr->sa_family) {
		case AF_INET:
			len = sizeof(sockaddr_in);
			this->addr.in = *reinterpret_cast<const sockaddr_in*>(addr);
			break;

		case AF_INET6:
			len = sizeof(sockaddr_in6);
			this->addr.in6 = *reinterpret_cast<const sockaddr_in6*>(addr);
			break;

		default:
			throw invalid_argument("Unsupported address family");
	}
}

SocketAddress SocketAddress::parse(const std::string& s, in_port_t port)
{
	if (s.find(':') != std::string::npos) {
		sockaddr_in6 addr;
		memset(&addr, 0, sizeof(addr));
		if (inet_pton(AF_INET6, s.c_str(), &addr.sin6_addr) < 0) {
			throw invalid_argument(string("Cannot parse IP address: ") + strerror(errno));
		}
		addr.sin6_port = htons(port);
		return SocketAddress(reinterpret_cast<sockaddr*>(&addr));
	} else {
		sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		if (inet_pton(AF_INET, s.c_str(), &addr.sin_addr) < 0) {
			throw invalid_argument(string("Cannot parse IP address: ") + strerror(errno));
		}
		addr.sin_port = htons(port);
		return SocketAddress(reinterpret_cast<sockaddr*>(&addr));
	}
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

