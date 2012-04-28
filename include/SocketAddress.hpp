#ifndef INCLUDE_SOCKETADDRESS_H
#define INCLUDE_SOCKETADDRESS_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <memory>
#include <cstring>
#include <boost/operators.hpp>

#include <IPAddress.hpp>

/**
 * Represents a socket address, composed of an IP address and a port number.
 */
class SocketAddress : boost::totally_ordered<SocketAddress> {
	private:
		union {
			sockaddr_in in;
			sockaddr_in6 in6;
		} addr_;

	public:
		/**
		 * Initializes a new SocketAddress from a given native representation.
		 * Supported address families for \a addr are AF_INET and AF_INET6.
		 */
		SocketAddress(const sockaddr* addr);						

		//! Initializes a new SocketAddress from a given IPAddress and port.
		SocketAddress(IPAddress&& addr, in_port_t port);

		//! Address family of the current instance.
		AddressFamily family() const
		{
			return static_cast<AddressFamily>(addr_.in.sin_family);
		}

		//! Native address family of the current instance (the AF_* stuff).
		int native_family() const
		{
			return addr_.in.sin_family;
		}

		//! Native representation of the current instance.
		const sockaddr* native() const
		{
			return reinterpret_cast<const sockaddr*>(&addr_.in);
		}

		//! Length of the native representation of the current instance, in bytes.
		socklen_t native_len() const
		{
			switch (family()) {
				case AddressFamily::IPv4:
					return sizeof(addr_.in);

				case AddressFamily::IPv6:
					return sizeof(addr_.in6);
			}
		}

		//! IP address of the current instance.
		IPAddress address() const
		{
			switch (family()) {
				case AddressFamily::IPv4:
					return IPAddress(addr_.in.sin_addr);

				case AddressFamily::IPv6:
					return IPAddress(addr_.in6.sin6_addr);
			}
		}

		//! Port of the current instance.
		in_port_t port() const
		{
			return addr_.in.sin_port;
		}


	public:
		//! Compares two SocketAddress instances for \e less-than.
		friend bool operator<(const SocketAddress& lhs, const SocketAddress& rhs)
		{
			return lhs.native_len() < rhs.native_len()
				? -1
				: std::memcmp(lhs.native(), rhs.native(), lhs.native_len());
		}

		//! Compares two SocketAddress instances for \e equals.
		friend bool operator==(const SocketAddress& lhs, const SocketAddress& rhs)
		{
			return lhs.family() == rhs.family()
				&& std::memcmp(lhs.native(), rhs.native(), lhs.native_len()) == 0;
		}
};

#endif
