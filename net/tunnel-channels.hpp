#ifndef NET_TUNNEL_CHANNELS_H
#define NET_TUNNEL_CHANNELS_H

#include <memory>
#include <ev++.h>
#include <boost/utility.hpp>

#include <signal.hpp>
#include <network.hpp>

#include "tunnel.hpp"

class Tunnel::Channel : boost::noncopyable {
	public:
		typedef Signal<void (Channel& sender, const Packet& packet)> OnReceive;
		typedef Signal<void (Channel& sender)> OnCanSend;

	protected:
		OnReceive receive;
		OnCanSend canSend;

		Link& link;
		uint8_t cid;

		Channel(Link& link, uint8_t cid);

	public:
		virtual ~Channel();

		virtual void readPacket(const Packet& packet) = 0;
		virtual ssize_t writePacket(const Packet& packet) = 0;

		virtual bs2::connection connectReceive(OnReceive::slot_function_type cb);
		virtual bs2::connection connectCanSend(OnCanSend::slot_function_type cb);
};	

class Tunnel::UnreliableChannel : public Channel {
	public:
		UnreliableChannel(Link& link, uint8_t cid);

		void readPacket(const Packet& packet) override;
		ssize_t writePacket(const Packet& packet) override;
};

class Tunnel::ReliableChannel : public Channel {
	private:
		ev::timer timeout;
		std::unique_ptr<Packet> inFlightPacket;
		uint32_t localSeq, peerSeq;

		void onTimeout(ev::timer& timer, int revents);

		void transmitQueue();
		void transmitPacket(const Packet& packet);

	public:
		ReliableChannel(Link& link, uint8_t cid, ev::loop_ref& loop);

		void readPacket(const Packet& packet) override;
		ssize_t writePacket(const Packet& packet) override;
};


#endif
