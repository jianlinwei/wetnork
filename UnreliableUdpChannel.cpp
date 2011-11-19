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
		UnreliableUdpPacketHeader(uint8_t cid)
			: _cid(cid)
		{}

		UnreliableUdpPacketHeader(const uint8_t* data)
			: _cid(data[0])
		{}

		uint8_t cid() const { return _cid; }

		void* data() { return &_cid; }
		size_t size() const { return sizeof(_cid); }
};

ssize_t UnreliableUdpChannel::send(const uint8_t* buffer, size_t len)
{
	UnreliableUdpPacketHeader header(cid);

	iovec iov[] = {
		{ header.data(), header.size() },
		{ const_cast<uint8_t*>(buffer), len }
	};

	msghdr msg;
	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	int result = parent.send(&msg, 0);
	if (result < len + sizeof(header)) {
		throw BadSend(strerror(errno));
	}

	return true;
}

void UnreliableUdpChannel::propagate(Packet packet)
{
	onReceive(Packet(packet.data(), sizeof(UnreliableUdpPacketHeader),
				packet.length() - sizeof(UnreliableUdpPacketHeader)));
}

