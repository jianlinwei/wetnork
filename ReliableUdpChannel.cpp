#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "network.hpp"
#include "network-udp-internal.hpp"

struct ReliableUdpPacketHeader {
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

		ReliableUdpPacketHeader(uint8_t cid, uint8_t flags, uint32_t seq)
		{
			header.cid = cid;
			header.flags = flags;
			header.seq = htonl(seq);
		}

		ReliableUdpPacketHeader(const uint8_t* buffer)
		{
			header = *reinterpret_cast<const header_t*>(buffer);
		}

		uint8_t cid() const { return header.cid; }
		uint8_t flags() const { return header.flags; }
		uint32_t seq() const { return ntohl(header.seq); }

		void* data() { return &header; }
};

ReliableUdpChannel::ReliableUdpChannel(UdpLink& parent, uint8_t cid, ev::loop_ref& loop)
	: UdpChannel(parent, cid), timeout(loop), inFlightPacket(NULL),
		localSeq(0), peerSeq(0)
{
	timeout.set<ReliableUdpChannel, &ReliableUdpChannel::onTimeout>(this);
}

void ReliableUdpChannel::onTimeout(ev::timer& timer, int revents)
{
	transmitQueue();
}

void ReliableUdpChannel::transmitQueue()
{
	if (inFlightPacket) {
		transmitPacket(*inFlightPacket);
		timeout.set(1, 0);
		timeout.start();
	}
}

void ReliableUdpChannel::transmitPacket(const Packet& packet)
{
	ReliableUdpPacketHeader header(cid, 0, localSeq);

	iovec iov[] = {
		{ header.data(), ReliableUdpPacketHeader::size },
		{ const_cast<uint8_t*>(packet.data()), packet.length() }
	};

	msghdr msg;
	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	int result = parent.send(&msg, 0);
	if (result < packet.length() + ReliableUdpPacketHeader::size) {
		throw BadSend(strerror(errno));
	}
}

ssize_t ReliableUdpChannel::send(const Packet& packet)
{
	if (inFlightPacket) {
		return false;
	}

	inFlightPacket = new Packet(packet);

	return true;
}

void ReliableUdpChannel::propagate(const Packet& packet)
{
	ReliableUdpPacketHeader header(packet.data());

	if (header.flags() & ReliableUdpPacketHeader::ACK
			&& localSeq == header.seq()) {
		localSeq++;
		delete inFlightPacket;
		inFlightPacket = NULL;

		onCanSend(*this);
	} else if (header.flags() == 0) {
		ReliableUdpPacketHeader ackHeader(cid, ReliableUdpPacketHeader::ACK, header.seq());

		if (peerSeq < header.seq()) {
			onReceive(*this, packet.skip(ReliableUdpPacketHeader::size));
		}

		peerSeq = peerSeq < header.seq() ? header.seq() : peerSeq;

		iovec iov[] = {
			{ ackHeader.data(), ReliableUdpPacketHeader::size }
		};

		msghdr msg;
		memset(&msg, 0, sizeof(msg));
		msg.msg_iov = iov;
		msg.msg_iovlen = 1;

		parent.send(&msg, 0);
	}
}

