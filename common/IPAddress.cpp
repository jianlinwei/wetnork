#include <IPAddress.hpp>

#include <stdexcept>
#include <arpa/inet.h>

IPAddress IPAddress::parse(const std::string& addr)
{
	if (addr.find(':') != std::string::npos) {
		in6_addr native;
		if (inet_pton(AF_INET6, addr.c_str(), &native) == 1) {
			return IPAddress(native);
		}
	} else {
		in_addr native;
		if (inet_pton(AF_INET, addr.c_str(), &native) == 1) {
			return IPAddress(native);
		}
	}

	throw std::invalid_argument("Cannot parse address");
}

std::string IPAddress::str() const
{
	char buffer[INET6_ADDRSTRLEN];
	return inet_ntop(native_family(), &addr_, buffer, sizeof(buffer));
}
