#ifndef NET_TUNNEL_H
#define NET_TUNNEL_H

#include <ev++.h>
#include <memory>
#include <map>

#include <signal.hpp>

#include "network.hpp"
#include "crypto.hpp"
#include "tun.hpp"

class Tunnel {
	private:
		class Channel;
		class UnreliableChannel;
		class ReliableChannel;

		CryptoSession link;
		ev::loop_ref& loop;

		std::unique_ptr<TunDevice> tun;

		static constexpr int DataChannelId = 0;
		std::unique_ptr<UnreliableChannel> dataChannel;

		Channel& getChannel(uint8_t id, bool reliable);
		void propagate(const Packet& packet);

		void tunRead(Stream& sender, const Packet& packet);
		void dataRead(Channel& sender, const Packet& packet);

	public:
		Tunnel(const Tunnel&) = delete;
		Tunnel& operator=(const Tunnel&) = delete;

		Tunnel(ev::loop_ref& loop, CryptoSession&& link, std::unique_ptr<TunDevice>&& tun);

		virtual ~Tunnel();
};

#endif
