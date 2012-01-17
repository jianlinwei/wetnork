#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "network.hpp"
#include "network-udp-internal.hpp"

UdpSocket::UdpSocket(int fd, ev::loop_ref& loop)
	: fd(fd), watcher(loop), loop(loop)
{
	watcher.set<UdpSocket, &UdpSocket::onPacketArrived>(this);
	watcher.start(fd, ev::READ);
}

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
	uint8_t pbuf[16];
	socklen_t addrlen = sizeof(addr);

	int plen = recvfrom(fd, pbuf, sizeof(pbuf), MSG_PEEK | MSG_TRUNC,
			reinterpret_cast<sockaddr*>(&addr), &addrlen);
	if (plen < 0) {
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

		link->onReceive(plen);
	}
}

UdpSocket* UdpSocket::create(const SocketAddress& addr, ev::loop_ref& loop)
{
	int fd = socket(addr.family(), SOCK_DGRAM, IPPROTO_UDP);

	if (fd < 0) {
		return NULL;
	}

	int err = bind(fd, addr.native(), addr.native_len());
	if (err < 0) {
		close(fd);
		throw BadAddress(strerror(errno));
	}

	return new UdpSocket(fd, loop);
}

boost::signals2::connection UdpSocket::listen(OnAccept::slot_function_type cb)
{
	return onAccept.connect(cb);
}
