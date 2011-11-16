#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "network.hpp"
#include "network-udp-internal.hpp"

void ConnectedUdpLink::onPacketArrived(ev::io& io, int revents)
{
	uint8_t pbuf[16];

	int plen = recv(fd, pbuf, sizeof(pbuf), MSG_PEEK | MSG_TRUNC);
	if (plen < 0) {
		// no error will happen here. trust me
		return;
	}

	onReceive(plen);
}

ssize_t ConnectedUdpLink::send(const msghdr* msg, int flags)
{
	return sendmsg(fd, msg, flags);
}

