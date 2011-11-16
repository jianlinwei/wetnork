#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "network.hpp"

SocketAddress::SocketAddress(sockaddr* addr)
{
	sockaddr_in* sin = reinterpret_cast<sockaddr_in*>(addr);

	switch (sin->sin_family) {
		case AF_INET:
			len = sizeof(sockaddr_in);
			break;

		case AF_INET6:
			len = sizeof(sockaddr_in6);
			break;

		default:
			throw bad_address();
	}

	memcpy(&this->addr, addr, len);
}

SocketAddress SocketAddress::parse(std::string addr, in_port_t port)
{
	SocketAddress result;

	memset(&result, 0, sizeof(result));

	if (addr.find(':') < addr.size()) {
		result.len = sizeof(sockaddr_in6);
		if (inet_pton(AF_INET6, addr.c_str(), &result.addr.in6.sin6_addr) < 0) {
			throw bad_address();
		}
		result.addr.in6.sin6_port = htons(port);
	} else {
		result.len = sizeof(sockaddr_in);
		if (inet_pton(AF_INET, addr.c_str(), &result.addr.in.sin_addr) < 0) {
			throw bad_address();
		}
		result.addr.in.sin_port = htons(port);
	}

	return result;
}






/* UdpChannel */

class UnreliableUdpChannel : public UdpChannel {
	public:
		UnreliableUdpChannel(UdpLink& parent, uint8_t cid)
			: UdpChannel(parent, cid)
		{}

		ssize_t send(const uint8_t* buffer, size_t len);
		void propagate(Packet packet);
};

ssize_t UnreliableUdpChannel::send(const uint8_t* buffer, size_t len)
{
	iovec iov[] = {
		{ &cid, sizeof(cid) },
		{ const_cast<uint8_t*>(buffer), len }
	};

	msghdr msg;
	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	int result = parent.send(&msg, 0);
	return result < 0 ? result : result - iov[0].iov_len;
}

void UnreliableUdpChannel::propagate(Packet packet)
{
	onReceive(Packet(packet.data(), 1, packet.length() - 1));
}




class ReliableUdpChannel : public UdpChannel {
	private:
		ev::timer timeout;
		std::queue<Packet> queue;
		uint32_t currentSeqNum;

		void onTimeout(ev::timer& timer, int revents);

	public:
		ReliableUdpChannel(UdpLink& parent, uint8_t cid, ev::loop_ref& loop)
			: UdpChannel(parent, cid), timeout(loop)
		{}

		ssize_t send(const uint8_t* buffer, size_t len);
		void propagate(Packet packet);
};

void ReliableUdpChannel::onTimeout(ev::timer& timer, int revents)
{
}

ssize_t ReliableUdpChannel::send(const uint8_t* buffer, size_t len)
{
}

void ReliableUdpChannel::propagate(Packet packet)
{
}




boost::signals::connection UdpChannel::connectReceive(OnReceive::slot_function_type cb)
{
	return onReceive.connect(cb);
}

/* UdpLink */

class AcceptedUdpLink : public UdpLink {
	private:
		SocketAddress peer;

	protected:
		AcceptedUdpLink(int fd, ev::loop_ref& loop, SocketAddress peer)
			: UdpLink(fd, loop), peer(peer)
		{}

	public:
		ssize_t send(const msghdr* msg, int flags);
};

ssize_t AcceptedUdpLink::send(const msghdr* msg, int flags)
{
	msghdr actual = *msg;
	actual.msg_name = const_cast<sockaddr*>(peer.native());
	actual.msg_namelen = peer.native_len();

	return sendmsg(fd, &actual, flags);
}



class ConnectedUdpLink : public UdpLink {
	private:
		ev::io watcher;

		void onPacketArrived(ev::io& io, int revents);

	protected:
		ConnectedUdpLink(int fd, ev::loop_ref& loop)
			: UdpLink(fd, loop), watcher(loop)
		{
			watcher.set<ConnectedUdpLink, &ConnectedUdpLink::onPacketArrived>(this);
			watcher.start(fd, ev::READ);
		}

	public:
		ssize_t send(const msghdr* msg, int flags);
};

void ConnectedUdpLink::onPacketArrived(ev::io& io, int revents)
{
	uint8_t pbuf[16];

	int plen = recv(fd, pbuf, sizeof(pbuf), MSG_PEEK | MSG_TRUNC);
	if (plen < 0) {
		// no error will happen here. trust me
		return;
	}

	onReceive(plen);
}

ssize_t ConnectedUdpLink::send(const msghdr* msg, int flags)
{
	return sendmsg(fd, msg, flags);
}




void UdpLink::onReceive(size_t size)
{
	if (size < 1) {
		// TODO: error handling
	}

	uint8_t* buffer = new uint8_t[size];

	int err = read(fd, buffer, size);
	if (err < 0) {
		// TODO: error handling
	}

	uint8_t channel = buffer[0];
	Packet packet(buffer, 0, size);

	if (channel & 0x80) {
		getChannel(channel & ~0x80, true)->propagate(packet);
	} else {
		getChannel(channel, false)->propagate(packet);
	}
}

UdpLink::~UdpLink()
{
	for (channel_map::iterator it = channels.begin(); it != channels.end(); it++) {
		delete it->second;
	}
}

UdpLink* UdpLink::connect(SocketAddress addr)
{
	// TODO: connect handling
}

UdpChannel* UdpLink::getChannel(int8_t id, bool reliable)
{
	if (id < 0) {
		// TODO: error handling
	}

	uint8_t cid = (reliable ? 0x80 : 0) | id;
	if (!channels.count(cid)) {
		if (reliable) {
			channels[cid] = new ReliableUdpChannel(*this, cid, loop);
		} else {
			channels[cid] = new UnreliableUdpChannel(*this, cid);
		}
	}
	return channels[cid];
}

boost::signals::connection UdpLink::connectClosed(OnClosed::slot_function_type cb)
{
	return onClosed.connect(cb);
}

void UdpLink::close()
{
	// TODO: close handling
}

/* UdpSocket */

UdpSocket::~UdpSocket()
{
	watcher.stop();

	for (peers_map::iterator it = peers.begin(); it != peers.end(); it++) {
		delete it->second;
	}
}

void UdpSocket::onPacketArrived(ev::io& io, int revents)
{
	sockaddr_storage addr;
	uint8_t pbuf[16];
	socklen_t addrlen = sizeof(addr);

	int plen = recvfrom(fd, pbuf, sizeof(pbuf), MSG_PEEK | MSG_TRUNC,
			reinterpret_cast<sockaddr*>(&addr), &addrlen);
	if (plen < 0) {
		// no error will happen here. trust me
		return;
	}

	SocketAddress s_addr(reinterpret_cast<sockaddr*>(&addr));

	if (peers.count(s_addr) == 0) {
		if (onAccept.num_slots()) {
			// TODO: accept connections
		} else {
			// drop packets nobody wants
			read(fd, pbuf, sizeof(pbuf));
			return;
		}
	} else {
		UdpLink* link = peers[s_addr];

		link->onReceive(plen);
	}
}

UdpSocket* UdpSocket::create(SocketAddress addr, ev::loop_ref& loop)
{
	int fd = socket(addr.family(), SOCK_DGRAM, IPPROTO_UDP);

	if (fd < 0) {
		return NULL;
	}

	int err = bind(fd, addr.native(), addr.native_len());
	if (err < 0) {
		close(fd);
		throw bad_address();
	}

	return new UdpSocket(fd, loop);
}

boost::signals::connection UdpSocket::listen(OnAccept::slot_function_type cb)
{
	return onAccept.connect(cb);
}

