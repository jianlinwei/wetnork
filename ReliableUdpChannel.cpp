#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "network.hpp"
#include "network-udp-internal.hpp"

struct ReliableUdpPacketHeader {
	private:
		uint8_t _cid;
		uint8_t _flags;
		uint32_t _seq;

	public:
		static const uint8_t ACK = 0x01;

		ReliableUdpPacketHeader(uint8_t cid, uint8_t flags, uint32_t seq)
			: _cid(cid), _flags(htonl(flags)), _seq(seq)
		{}

		uint8_t cid() const { return _cid; }
		uint8_t flags() const { return _flags; }
		uint32_t seq() const { return ntohl(_seq); }
};

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
		{ &header, sizeof(header) },
		{ const_cast<uint8_t*>(packet.data()), packet.length() }
	};

	msghdr msg;
	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	int result = parent.send(&msg, 0);
	if (result < packet.length() + sizeof(header)) {
		throw bad_packet();
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
	const ReliableUdpPacketHeader* header =
		reinterpret_cast<const ReliableUdpPacketHeader*>(packet.data());

	if (header->flags() & ReliableUdpPacketHeader::ACK
			&& localSeq == header->seq()) {
		localSeq++;
		delete inFlightPacket;
		inFlightPacket = NULL;

		onCanSend();
	} else if (header->flags() == 0) {
		ReliableUdpPacketHeader ackHeader(cid, ReliableUdpPacketHeader::ACK, header->seq());

		if (peerSeq < header->seq()) {
			onReceive(Packet(packet.data(), sizeof(ackHeader),
						packet.length() - sizeof(ackHeader)));
		}

		peerSeq = peerSeq < header->seq() ? header->seq() : peerSeq;

		iovec iov[] = {
			{ &ackHeader, sizeof(ackHeader) }
		};

		msghdr msg;
		memset(&msg, 0, sizeof(msg));
		msg.msg_iov = iov;
		msg.msg_iovlen = 1;

		parent.send(&msg, 0);
	}
}

