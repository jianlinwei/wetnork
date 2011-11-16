#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "network.hpp"
#include "network-udp-internal.hpp"

struct UnreliableUdpPacketHeader {
	private:
		uint8_t _cid;

	public:
		UnreliableUdpPacketHeader(uint8_t cid)
			: _cid(cid)
		{}

		uint8_t cid() const { return _cid; }
};

ssize_t UnreliableUdpChannel::send(const uint8_t* buffer, size_t len)
{
	UnreliableUdpPacketHeader header(cid);

	iovec iov[] = {
		{ &header, sizeof(header) },
		{ const_cast<uint8_t*>(buffer), len }
	};

	msghdr msg;
	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	int result = parent.send(&msg, 0);
	if (result < len + sizeof(header)) {
		throw bad_packet();
	}

	return true;
}

void UnreliableUdpChannel::propagate(Packet packet)
{
	onReceive(Packet(packet.data(), sizeof(UnreliableUdpPacketHeader),
				packet.length() - sizeof(UnreliableUdpPacketHeader)));
}

