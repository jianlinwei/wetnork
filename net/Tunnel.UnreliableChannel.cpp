#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>

#include "tunnel-channels.hpp"

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

		operator Packet() const
		{
			return Packet(&_cid, 0, size, Packet::NoCapture);
		}
};
		
Tunnel::UnreliableChannel::UnreliableChannel(Link& link, uint8_t cid)
	: Channel(link, cid)
{
}

ssize_t Tunnel::UnreliableChannel::writePacket(const Packet& packet)
{
	UnreliablePacketHeader header(cid);

	return link.write(header, packet);
}

void Tunnel::UnreliableChannel::readPacket(const Packet& packet)
{
	receive(*this, packet.skip(UnreliablePacketHeader::size));
}

