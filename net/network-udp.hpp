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

class UdpLink;
class UdpSocket;

class UdpLink : public Link {
	friend class UdpSocket;
	private:
		ev::loop_ref& loop;
		SocketAddress peer;
		UdpSocket& parent;

	protected:
		int fd;

		UdpLink(UdpSocket& parent, int fd, const SocketAddress& peer, ev::loop_ref& loop);

		void propagatePacket(const Packet& packet);

	public:
		~UdpLink() override;

		bs2::connection connectStateChanged(OnStateChanged::slot_function_type cb) override;

		ssize_t send(const msghdr* msg) override;

		void close() override;
};

class UdpSocket : public Socket, boost::noncopyable {
	friend class UdpLink;
	private:
		typedef std::map<SocketAddress, UdpLink*> peers_map;

		int fd;
		ev::io watcher;
		peers_map peers;
		ev::loop_ref& loop;
		SocketAddress _address;

		void onPacketArrived(ev::io& io, int revents);

		UdpSocket(int fd, const SocketAddress& address, ev::loop_ref& loop);

		void removeLink(const SocketAddress& link);

	public:
		~UdpSocket() override;

		static UdpSocket* create(const SocketAddress& addr, ev::loop_ref& loop);

		UdpLink* connect(const SocketAddress& peer) override;

		const SocketAddress& address() const override;
};

#endif
