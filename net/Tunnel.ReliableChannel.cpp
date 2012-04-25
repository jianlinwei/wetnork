#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>

#include "tunnel-channels.hpp"

using std::string;

struct ReliablePacketHeader {
	private:
		struct header_t {
			uint8_t cid;
			uint8_t flags;
			uint32_t seq;
		} __attribute__ ((packed));

		header_t header;

	public:
		static const uint8_t ACK = 0x01;
		static const size_t size = sizeof(header_t);

		ReliablePacketHeader(uint8_t cid, uint8_t flags, uint32_t seq)
		{
			header.cid = cid;
			header.flags = flags;
			header.seq = htonl(seq);
		}

		ReliablePacketHeader(const uint8_t* buffer)
		{
			header = *reinterpret_cast<const header_t*>(buffer);
		}

		uint8_t cid() const { return header.cid; }
		uint8_t flags() const { return header.flags; }
		uint32_t seq() const { return ntohl(header.seq); }

		operator Packet() const
		{
			return Packet(reinterpret_cast<const uint8_t*>(&header), 0, size, Packet::NoCapture);
		}
};

Tunnel::ReliableChannel::ReliableChannel(Stream& next, uint8_t cid, ev::loop_ref& loop)
	: Channel(next, cid), timeout(loop), inFlightPacket(), localSeq(0), peerSeq(0)
{
	timeout.set<ReliableChannel, &ReliableChannel::onTimeout>(this);
}

void Tunnel::ReliableChannel::onTimeout(ev::timer& timer, int revents)
{
	transmitQueue();
}

void Tunnel::ReliableChannel::transmitQueue()
{
	if (inFlightPacket) {
		transmitPacket(*inFlightPacket);
		timeout.set(1, 0);
		timeout.start();
	}
}

void Tunnel::ReliableChannel::transmitPacket(const Packet& packet)
{
	ReliablePacketHeader header(cid, 0, localSeq);

	next.write(header, packet);
}

ssize_t Tunnel::ReliableChannel::writePacket(const Packet& packet)
{
	if (inFlightPacket) {
		return false;
	}

	inFlightPacket.reset(new Packet(packet));

	return true;
}

void Tunnel::ReliableChannel::readPacket(const Packet& packet)
{
	ReliablePacketHeader header(packet.data());

	if (header.flags() & ReliablePacketHeader::ACK && localSeq == header.seq()) {
		localSeq++;
		inFlightPacket.reset();
		timeout.stop();

		canSend(*this);
	} else if (header.flags() == 0) {
		ReliablePacketHeader ackHeader(cid, ReliablePacketHeader::ACK, header.seq());

		if (peerSeq < header.seq()) {
			peerSeq = header.seq();
			receive(*this, packet.skip(ReliablePacketHeader::size));
		}

		next.write(ackHeader);
	}
}

