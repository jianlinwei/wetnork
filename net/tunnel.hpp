#ifndef NET_TUNNEL_H
#define NET_TUNNEL_H

#include <ev++.h>
#include <memory>
#include <map>

#include <signal.hpp>

#include "network.hpp"

class Tunnel {
	private:
		class Channel;
		class UnreliableChannel;
		class ReliableChannel;

		typedef std::map<uint8_t, std::unique_ptr<Channel>> channel_map;

		channel_map channels;
		std::unique_ptr<Link> link;
		ev::loop_ref& loop;

		Channel& getChannel(int8_t id, bool reliable);
		void propagate(const Packet& packet);

	public:
		Tunnel(const Tunnel&) = delete;
		Tunnel& operator=(const Tunnel&) = delete;

		Tunnel(ev::loop_ref& loop, Link* link);

		virtual ~Tunnel();
};

#endif
