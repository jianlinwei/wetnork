#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "tunnel.hpp"

using std::string;

struct ReliablePacketHeader {
	private:
		typedef struct {
			uint8_t cid;
			uint8_t flags;
			uint32_t seq;
		} header_t;

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

		void* data() { return &header; }
};

Tunnel::ReliableChannel::ReliableChannel(Link& link, uint8_t cid, ev::loop_ref& loop)
	: Channel(link, cid), timeout(loop), inFlightPacket(NULL),
		localSeq(0), peerSeq(0)
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

	iovec iov[] = {
		{ header.data(), ReliablePacketHeader::size },
		{ const_cast<uint8_t*>(packet.data()), packet.length() }
	};

	msghdr msg;
	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	int result = link.send(&msg);
	if (result < packet.length() + ReliablePacketHeader::size
			&& !(result == -1 && errno == EAGAIN)) {
		throw SocketException(errno, string("Could not send packet: ") + strerror(errno));
	}
}

ssize_t Tunnel::ReliableChannel::send(const Packet& packet)
{
	if (inFlightPacket) {
		return false;
	}

	inFlightPacket = new Packet(packet);

	return true;
}

void Tunnel::ReliableChannel::propagate(const Packet& packet)
{
	ReliablePacketHeader header(packet.data());

	if (header.flags() & ReliablePacketHeader::ACK
			&& localSeq == header.seq()) {
		localSeq++;
		delete inFlightPacket;
		inFlightPacket = NULL;

		canSend(*this);
	} else if (header.flags() == 0) {
		ReliablePacketHeader ackHeader(cid, ReliablePacketHeader::ACK, header.seq());

		if (peerSeq < header.seq()) {
			receive(*this, packet.skip(ReliablePacketHeader::size));
		}

		peerSeq = peerSeq < header.seq() ? header.seq() : peerSeq;

		iovec iov[] = {
			{ ackHeader.data(), ReliablePacketHeader::size }
		};

		msghdr msg;
		memset(&msg, 0, sizeof(msg));
		msg.msg_iov = iov;
		msg.msg_iovlen = 1;

		link.send(&msg);
	}
}

