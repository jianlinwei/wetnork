#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "network.hpp"
#include "network-udp-internal.hpp"

ssize_t AcceptedUdpLink::send(const msghdr* msg, int flags)
{
	msghdr actual = *msg;
	actual.msg_name = const_cast<sockaddr*>(peer.native());
	actual.msg_namelen = peer.native_len();

	return sendmsg(fd, &actual, flags);
}

