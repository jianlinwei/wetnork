#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "network.hpp"
#include "network-udp-internal.hpp"

struct UnreliableUdpPacketHeader {
	private:
		uint8_t _cid;

	public:
		static const size_t size = sizeof(uint8_t);

		UnreliableUdpPacketHeader(uint8_t cid)
			: _cid(cid)
		{}

		UnreliableUdpPacketHeader(const uint8_t* data)
			: _cid(data[0])
		{}

		uint8_t cid() const { return _cid; }

		void* data() { return &_cid; }
};
		
UnreliableUdpChannel::UnreliableUdpChannel(UdpLink& parent, uint8_t cid)
	: UdpChannel(parent, cid)
{
}

ssize_t UnreliableUdpChannel::send(const Packet& packet)
{
	UnreliableUdpPacketHeader header(cid);

	iovec iov[] = {
		{ header.data(), UnreliableUdpPacketHeader::size },
		{ const_cast<uint8_t*>(packet.data()), packet.length() }
	};

	msghdr msg;
	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	int result = parent.send(&msg);
	if (result < packet.length() + UnreliableUdpPacketHeader::size) {
		throw BadSend(strerror(errno));
	}

	return true;
}

void UnreliableUdpChannel::propagate(const Packet& packet)
{
	onReceive(*this, packet.skip(UnreliableUdpPacketHeader::size));
}

