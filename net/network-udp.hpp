#ifndef NET_NETWORK_UDP_H
#define NET_NETWORK_UDP_H

#include <sys/socket.h>
#include <ev++.h>
#include <netinet/in.h>
#include <cstdint>
#include <string>
#include <arpa/inet.h>
#include <map>
#include <memory>

#include "network.hpp"

class UdpLink;
class UdpSocket;

class UdpLink : public Link {
	friend class UdpSocket;
	private:
		SocketAddress peer;
		UdpSocket& parent;

	protected:
		int fd;

		UdpLink(UdpSocket& parent, int fd, const SocketAddress& peer);

		void propagate(const Packet& packet) override;

	public:
		UdpLink(const UdpLink&) = delete;
		UdpLink& operator=(const UdpLink&) = delete;

		~UdpLink() override;

		ms::connection connectStateChanged(OnStateChanged::slot_function_type cb) override;

		bool write(const Packet& packet) override;

		void close() override;
};

class UdpSocket : public Socket {
	friend class UdpLink;
	private:
		typedef std::map<SocketAddress, std::unique_ptr<UdpLink>> peers_map;

		int fd;
		ev::io watcher;
		peers_map peers;
		ev::loop_ref& loop;
		SocketAddress _address;

		void onPacketArrived(ev::io& io, int revents);

		UdpSocket(int fd, const SocketAddress& address, ev::loop_ref& loop);

		void removeLink(const SocketAddress& link);

	public:
		UdpSocket(const UdpSocket&) = delete;
		UdpSocket& operator=(const UdpSocket&) = delete;

		~UdpSocket() override;

		static UdpSocket* create(const SocketAddress& addr, ev::loop_ref& loop);

		UdpLink& connect(const SocketAddress& peer) override;

		const SocketAddress& address() const override;
};

#endif
