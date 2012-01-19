#ifndef NETWORK_H
#define NETWORK_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <string>

struct SocketAddress {
	private:
		union {
			sockaddr_in in;
			sockaddr_in6 in6;
		} addr;
		socklen_t len;

		SocketAddress() = delete;

	public:
		SocketAddress(const sockaddr* addr);						

		static SocketAddress parse(const std::string& addr, in_port_t port);

		int family() const;

		const sockaddr* native() const;

		socklen_t native_len() const;

		bool operator<(const SocketAddress& other) const;

		bool operator==(const SocketAddress& other) const;
};

#endif
