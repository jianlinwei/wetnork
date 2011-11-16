#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "network.hpp"
#include "network-udp-internal.hpp"

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

UdpLink* UdpLink::connect(SocketAddress addr)
{
	// TODO: connect handling
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

boost::signals::connection UdpLink::connectClosed(OnClosed::slot_function_type cb)
{
	return onClosed.connect(cb);
}

void UdpLink::close()
{
	// TODO: close handling
}

