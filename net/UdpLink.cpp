#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "network.hpp"
#include "network-udp-internal.hpp"

using namespace std;

UdpLink::UdpLink(const SocketAddress& peer, ev::loop_ref& loop)
	: loop(loop), peer(peer)
{
	connect(nullptr, peer);
}

UdpLink::UdpLink(const SocketAddress& local, const SocketAddress& peer, ev::loop_ref& loop)
	: loop(loop), peer(peer)
{
	connect(&local, peer);
}

UdpLink::UdpLink(int fd, const SocketAddress& peer, ev::loop_ref& loop)
	: loop(loop), peer(peer), fd(fd)
{
	_state = LinkState::Open;
}

void UdpLink::connect(const SocketAddress* local, const SocketAddress& peer)
{
	fd = socket(peer.family(), SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		switch (errno) {
			case EACCES:
			case EAFNOSUPPORT:
			case EINVAL:
			case EPROTONOSUPPORT:
				throw BadAddress(string("Could not open socket: ") + strerror(errno));

			default:
				throw InvalidOperation(string("Could not open socket: ") + strerror(errno));
		}
	}

	if (local) {
		if (bind(fd, local->native(), local->native_len()) < 0) {
			switch (errno) {
				case EACCES:
				case EADDRINUSE:
					throw BadAddress(string("Could not bind to local address: ") + strerror(errno));

				default:
					throw InvalidOperation(string("Could not bind to local address: ") + strerror(errno));
			}
		}
	}

	_state = LinkState::Opening;

	// TODO: establish link
}

void UdpLink::onReceive(size_t size)
{
	if (size < 1) {
		// TODO: error handling
	}

	uint8_t* buffer = new uint8_t[size];

	int err = read(fd, buffer, size);
	if (err < 0) {
		// TODO: error handling
	}

	uint8_t channel = buffer[0];
	Packet packet(buffer, 0, size);

	if (channel & 0x80) {
		getChannel(channel & ~0x80, true)->propagate(packet);
	} else {
		getChannel(channel, false)->propagate(packet);
	}
}

UdpLink::~UdpLink()
{
	for (channel_map::iterator it = channels.begin(); it != channels.end(); it++) {
		delete it->second;
	}
}

ssize_t UdpLink::send(const msghdr* msg, int flags)
{
	msghdr actual = *msg;
	actual.msg_name = const_cast<sockaddr*>(peer.native());
	actual.msg_namelen = peer.native_len();

	return sendmsg(fd, &actual, flags);
}

UdpChannel* UdpLink::getChannel(int8_t id, bool reliable)
{
	if (id < 0) {
		// TODO: error handling
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

