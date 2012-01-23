#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>

#include "network.hpp"
#include "network-udp-internal.hpp"

using namespace std;

UdpLink::UdpLink(int fd, const SocketAddress& peer, ev::loop_ref& loop)
	: loop(loop), peer(peer), fd(fd)
{
	_state = LinkState::Open;
}

void UdpLink::propagatePacket(const Packet& packet)
{
	if (packet.length() == 0) {
		throw InvalidOperation("Packet too short");
	}

	uint8_t channel = packet.data()[0];
	bool reliableChannel = !!(channel & 0x80);

	getChannel(channel & ~0x80, reliableChannel)->propagate(packet.skip(1));
}

UdpLink::~UdpLink()
{
	for (channel_map::iterator it = channels.begin(); it != channels.end(); it++) {
		delete it->second;
	}
}

ssize_t UdpLink::send(const msghdr* msg)
{
	SocketAddress peer = this->peer;

	msghdr actual = *msg;
	actual.msg_name = const_cast<sockaddr*>(peer.native());
	actual.msg_namelen = peer.native_len();

	return sendmsg(fd, &actual, MSG_DONTWAIT);
}

UdpChannel* UdpLink::getChannel(int8_t id, bool reliable)
{
	if (id < 0) {
		throw invalid_argument("id");
	}

	uint8_t cid = (reliable ? 0x80 : 0) | id;
	if (!channels.count(cid)) {
		if (reliable) {
			channels[cid] = new ReliableUdpChannel(*this, cid, loop);
		} else {
			channels[cid] = new UnreliableUdpChannel(*this, cid);
		}
	}
	return channels[cid];
}

void UdpLink::close()
{
	// TODO: close handling
}

