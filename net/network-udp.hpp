#ifndef NET_NETWORK_UDP_H
#define NET_NETWORK_UDP_H

#include <sys/socket.h>
#include <ev++.h>
#include <netinet/in.h>
#include <boost/signals2.hpp>
#include <map>
#include <stdint.h>
#include <string>
#include <arpa/inet.h>
#include <boost/shared_array.hpp>
#include <boost/function.hpp>

#include "network.hpp"

class UdpChannel;
class UdpLink;
class UdpSocket;

class UdpChannel : public Channel {
	friend class UdpLink;
	protected:
		OnReceive onReceive;
		OnCanSend onCanSend;
		UdpLink& parent;
		uint8_t cid;

		UdpChannel(UdpLink& parent, uint8_t cid);

		virtual void propagate(const Packet& packet) = 0;

	public:
		ssize_t send(const Packet& packet) = 0;

		SignalConnection connectReceive(OnReceive::slot_function_type cb);

		SignalConnection connectCanSend(OnCanSend::slot_function_type cb);
};

class UdpLink : public Link {
	friend class UdpChannel;
	friend class UnreliableUdpChannel;
	friend class ReliableUdpChannel;
	friend class UdpSocket;
	private:
		typedef std::map<uint8_t, UdpChannel*> channel_map;

		channel_map channels;
		ev::loop_ref& loop;
		SocketAddress peer;

	protected:
		int fd;

		UdpLink(int fd, const SocketAddress& peer, ev::loop_ref& loop);

		void onReceive(size_t size);

		ssize_t send(const msghdr* msg, int flags);

	public:
		~UdpLink() override;

		UdpChannel* getChannel(int8_t id, bool reliable) override;

		void close() override;
};

class UdpSocket : public Socket, public boost::noncopyable {
	private:
		typedef std::map<SocketAddress, UdpLink*> peers_map;

		int fd;
		ev::io watcher;
		peers_map peers;
		OnAccept onAccept;
		ev::loop_ref& loop;

		void onPacketArrived(ev::io& io, int revents);

		UdpSocket(int fd, ev::loop_ref& loop);

	public:
		~UdpSocket() override;

		static UdpSocket* create(const SocketAddress& addr, ev::loop_ref& loop);

		UdpLink* connect(const SocketAddress& peer) override;

		SignalConnection listen(OnAccept::slot_function_type cb) override;
};

#endif

