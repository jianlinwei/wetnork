#include <IPAddress.hpp>

#include <stdexcept>
#include <arpa/inet.h>
#include <ipc/serialization.hpp>

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

void serialize(const IPAddress& ip, ipc::Serializer& s)
{
	s.write(ip.family_);
	switch (ip.family_) {
		case AddressFamily::IPv4:
			s.write(ip.addr_.in);
			break;

		case AddressFamily::IPv6:
			s.write(ip.addr_.in6);
			break;
	}
}

void deserialize(IPAddress* result, ipc::Deserializer& d)
{
	auto family = d.read<AddressFamily>();
	switch (family) {
		case AddressFamily::IPv4:
			new (result) IPAddress(d.read<in_addr>());
			break;

		case AddressFamily::IPv6:
			new (result) IPAddress(d.read<in6_addr>());
			break;
	}
}
