#ifndef NETWORK_UDP_H
#define NETWORK_UDP_H

#include <sys/socket.h>
#include <ev++.h>
#include <netinet/in.h>
#include <boost/signal.hpp>
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

		virtual void propagate(Packet packet) = 0;

	public:
		ssize_t send(const uint8_t* buffer, size_t len) = 0;

		boost::signals::connection connectReceive(OnReceive::slot_function_type cb);

		boost::signals::connection connectCanSend(OnCanSend::slot_function_type cb);
};

class UdpLink : public Link {
	friend class UdpChannel;
	friend class UnreliableUdpChannel;
	friend class ReliableUdpChannel;
	friend class UdpSocket;
	private:
		typedef std::map<uint8_t, UdpChannel*> channel_map;

		OnClosed onClosed;
		channel_map channels;
		ev::loop_ref& loop;
		SocketAddress peer;

	protected:
		int fd;

		UdpLink(int fd, SocketAddress& peer, ev::loop_ref& loop);

		void onReceive(size_t size);

		ssize_t send(const msghdr* msg, int flags);

	public:
		~UdpLink();

		static UdpLink* connect(SocketAddress addr);

		UdpChannel* getChannel(int8_t id, bool reliable);

		boost::signals::connection connectClosed(OnClosed::slot_function_type cb);

		void close();
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
		~UdpSocket();

		static UdpSocket* create(SocketAddress addr, ev::loop_ref& loop);

		boost::signals::connection listen(OnAccept::slot_function_type cb);

		void connect(boost::function<void (UdpLink* link)> cb);
};

#endif

