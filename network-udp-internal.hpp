#ifndef NETWORK_UDP_INTERNAL
#define NETWORK_UDP_INTERNAL

#include "network.hpp"
#include "network-udp.hpp"

class UnreliableUdpChannel : public UdpChannel {
	public:
		UnreliableUdpChannel(UdpLink& parent, uint8_t cid);

		ssize_t send(const uint8_t* buffer, size_t len);
		void propagate(Packet packet);
};

class ReliableUdpChannel : public UdpChannel {
	private:
		ev::timer timeout;
		Packet* inFlightPacket;
		uint32_t localSeq, peerSeq;

		void onTimeout(ev::timer& timer, int revents);

		void transmitQueue();
		void transmitPacket(Packet packet);

	public:
		ReliableUdpChannel(UdpLink& parent, uint8_t cid, ev::loop_ref& loop);

		ssize_t send(const uint8_t* buffer, size_t len);
		void propagate(Packet packet);
};

#endif
