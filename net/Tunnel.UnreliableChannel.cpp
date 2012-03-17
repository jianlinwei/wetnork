#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "tunnel.hpp"

using namespace std;

struct UnreliablePacketHeader {
	private:
		uint8_t _cid;

	public:
		static const size_t size = sizeof(uint8_t);

		UnreliablePacketHeader(uint8_t cid)
			: _cid(cid)
		{}

		UnreliablePacketHeader(const uint8_t* data)
			: _cid(data[0])
		{}

		uint8_t cid() const { return _cid; }

		void* data() { return &_cid; }
};
		
Tunnel::UnreliableChannel::UnreliableChannel(Link& link, uint8_t cid)
	: Channel(link, cid)
{
}

ssize_t Tunnel::UnreliableChannel::writePacket(const Packet& packet)
{
	UnreliablePacketHeader header(cid);

	iovec iov[] = {
		{ header.data(), UnreliablePacketHeader::size },
		{ const_cast<uint8_t*>(packet.data()), packet.length() }
	};

	msghdr msg;
	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	int result = link.send(&msg);
	if (result < packet.length() + UnreliablePacketHeader::size
			&& !(result == -1 && errno == EAGAIN)) {
		throw SocketException(errno, string("Could not send packet: ") + strerror(errno));
	}

	return true;
}

void Tunnel::UnreliableChannel::readPacket(const Packet& packet)
{
	receive(*this, packet.skip(UnreliablePacketHeader::size));
}

