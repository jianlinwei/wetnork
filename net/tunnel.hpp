#ifndef NET_TUNNEL_H
#define NET_TUNNEL_H

#include <boost/utility.hpp>
#include "signal.hpp"
#include "network.hpp"
#include "network-common.hpp"
#include <ev++.h>
#include <map>

class Tunnel : private boost::noncopyable {
	private:
		class Channel {
			friend class Tunnel;
			public:
				typedef Signal<void (Channel& sender, const Packet& packet)> OnReceive;
				typedef Signal<void (Channel& sender)> OnCanSend;

			protected:
				OnReceive receive;
				OnCanSend canSend;

				Link& link;
				uint8_t cid;

				Channel(Link& link, uint8_t cid);

				virtual void propagate(const Packet& packet) = 0;

			public:
				virtual ~Channel();

				virtual ssize_t send(const Packet& packet) = 0;

				virtual bs2::connection connectReceive(OnReceive::slot_function_type cb);

				virtual bs2::connection connectCanSend(OnCanSend::slot_function_type cb);
		};	

		class UnreliableChannel : public Channel {
			public:
				UnreliableChannel(Link& link, uint8_t cid);

				ssize_t send(const Packet& packet) override;
				void propagate(const Packet& packet) override;
		};

		class ReliableChannel : public Channel {
			private:
				ev::timer timeout;
				Packet* inFlightPacket;
				uint32_t localSeq, peerSeq;

				void onTimeout(ev::timer& timer, int revents);

				void transmitQueue();
				void transmitPacket(const Packet& packet);

			public:
				ReliableChannel(Link& link, uint8_t cid, ev::loop_ref& loop);

				ssize_t send(const Packet& packet) override;
				void propagate(const Packet& packet) override;
		};

		typedef std::map<uint8_t, Channel*> channel_map;

		channel_map channels;
		Link* link;
		ev::loop_ref& loop;

		Channel& getChannel(int8_t id, bool reliable);
		void propagate(const Packet& packet);

	public:
		Tunnel(ev::loop_ref& loop, Link* link);

		virtual ~Tunnel();
};

#endif
