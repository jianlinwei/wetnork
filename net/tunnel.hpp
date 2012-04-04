#ifndef NET_TUNNEL_H
#define NET_TUNNEL_H

#include <boost/ptr_container/ptr_map.hpp>
#include <ev++.h>
#include <memory>

#include <signal.hpp>

#include "network.hpp"

class Tunnel {
	private:
		class Channel;
		class UnreliableChannel;
		class ReliableChannel;

		typedef boost::ptr_map<uint8_t, Channel> channel_map;

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
