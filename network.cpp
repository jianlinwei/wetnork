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





/* UdpSocket */

UdpSocket::~UdpSocket()
{
	watcher.stop();

	for (peers_map::iterator it = peers.begin(); it != peers.end(); it++) {
		delete it->second;
	}
}

void UdpSocket::onPacketArrived(ev::io& io, int revents)
{
	sockaddr_storage addr;
	char pbuf[16];
	socklen_t addrlen = sizeof(addr);

	int err = recvfrom(fd, pbuf, sizeof(pbuf), MSG_PEEK,
			reinterpret_cast<sockaddr*>(&addr), &addrlen);
	if (err < 0) {
		// no error will happen here. trust me
		return;
	}

	SocketAddress s_addr(reinterpret_cast<sockaddr*>(&addr));

	if (peers.count(s_addr) == 0) {
		if (onAccept.num_slots()) {
			// TODO: accept connections
		} else {
			// drop packets nobody wants
			read(fd, pbuf, sizeof(pbuf));
			return;
		}
	} else {
		UdpLink* link = peers[s_addr];

		// TODO: tell link that a packet was received
	}
}

UdpSocket* UdpSocket::create(SocketAddress addr, ev_loop* loop)
{
	int fd = socket(addr.family(), SOCK_DGRAM, IPPROTO_UDP);

	if (fd < 0) {
		return NULL;
	}

	int err = bind(fd, addr.native(), addr.native_len());
	if (err < 0) {
		close(fd);
		throw bad_address();
	}

	return new UdpSocket(fd, loop);
}

boost::signals::connection UdpSocket::listen(OnAccept::slot_function_type cb)
{
	return onAccept.connect(cb);
}

