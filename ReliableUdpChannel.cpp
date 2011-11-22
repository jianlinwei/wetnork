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
		size_t size() const { return sizeof(header); }
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

void ReliableUdpChannel::transmitPacket(Packet packet)
{
	ReliableUdpPacketHeader header(cid, 0, localSeq);

	iovec iov[] = {
		{ header.data(), header.size() },
		{ const_cast<uint8_t*>(packet.data()), packet.length() }
	};

	msghdr msg;
	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	int result = parent.send(&msg, 0);
	if (result < packet.length() + sizeof(header)) {
		throw BadSend(strerror(errno));
	}
}

ssize_t ReliableUdpChannel::send(const uint8_t* buffer, size_t len)
{
	if (inFlightPacket) {
		return false;
	}

	uint8_t* pbuffer = new uint8_t[len];
	memcpy(pbuffer, buffer, len);
	inFlightPacket = new Packet(pbuffer, 0, len);

	return true;
}

void ReliableUdpChannel::propagate(Packet packet)
{
	ReliableUdpPacketHeader header(packet.data());

	if (header.flags() & ReliableUdpPacketHeader::ACK
			&& localSeq == header.seq()) {
		localSeq++;
		delete inFlightPacket;
		inFlightPacket = NULL;

		onCanSend();
	} else if (header.flags() == 0) {
		ReliableUdpPacketHeader ackHeader(cid, ReliableUdpPacketHeader::ACK, header.seq());

		if (peerSeq < header.seq()) {
			onReceive(Packet(packet.data(), sizeof(ackHeader),
						packet.length() - sizeof(ackHeader)));
		}

		peerSeq = peerSeq < header.seq() ? header.seq() : peerSeq;

		iovec iov[] = {
			{ ackHeader.data(), ackHeader.size() }
		};

		msghdr msg;
		memset(&msg, 0, sizeof(msg));
		msg.msg_iov = iov;
		msg.msg_iovlen = 1;

		parent.send(&msg, 0);
	}
}

