#ifndef LINK_H
#define LINK_H

#include <sys/socket.h>
#include <ev++.h>
#include <netinet/in.h>
#include <boost/signal.hpp>
#include <map>
#include <stdint.h>
#include <string>
#include <arpa/inet.h>
#include <boost/shared_array.hpp>




class bad_address {};

struct SocketAddress {
	private:
		union {
			sockaddr_in in;
			sockaddr_in6 in6;
		} addr;
		socklen_t len;

		SocketAddress()
		{}

	public:
		SocketAddress(sockaddr* addr);						

		static SocketAddress parse(std::string addr, in_port_t port);

		int family() const { return addr.in.sin_family; }

		const sockaddr* native() const { return reinterpret_cast<const sockaddr*>(&addr); }

		socklen_t native_len() const { return len; }

		bool operator<(const SocketAddress& other) const
		{
			return len < other.len
				? -1
				: memcmp(&addr, &other.addr, len);
		}

		bool operator==(const SocketAddress& other) const
		{
			return len == other.len
				&& memcmp(&addr, &other.addr, len) == 0;
		}
};





class Packet {
	private:
		boost::shared_array<const uint8_t> _data;
		off_t _offset;
		size_t _length;

	public:
		Packet(const uint8_t* data, off_t offset, size_t length)
			: _data(data), _offset(offset), _length(length)
		{}

		const uint8_t* data() const { return _data.get() + _offset; }

		size_t length() const { return _length; }
};

class Channel {
	protected:
		typedef boost::signal<void (Packet packet)> OnReceive;

	public:
		virtual ssize_t send(const uint8_t* buffer, size_t len) = 0;

		virtual boost::signals::connection connectReceive(OnReceive::slot_function_type cb) = 0;
};

class Link {
	protected:
		typedef boost::signal<void ()> OnClosed;

	public:
		virtual Channel* getChannel(int8_t id, bool reliable) = 0;

		virtual boost::signals::connection connectClosed(OnClosed::slot_function_type cb) = 0;

		virtual void close() = 0;
};

class Socket {
	protected:
		typedef boost::signal<void (Link*)> OnAccept;

		Socket()
		{}

	public:
		virtual boost::signals::connection listen(OnAccept::slot_function_type cb) = 0;
};







class UdpChannel;
class UdpLink;
class UdpSocket;

class UdpChannel : public Channel {
	friend class UdpLink;
	protected:
		OnReceive onReceive;
		UdpLink& parent;
		uint8_t cid;

		UdpChannel(UdpLink& parent, uint8_t cid)
			: parent(parent), cid(cid)
		{}

		virtual void propagate(Packet packet) = 0;

	public:
		ssize_t send(const uint8_t* buffer, size_t len) = 0;

		boost::signals::connection connectReceive(OnReceive::slot_function_type cb);
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

	protected:
		int fd;

		UdpLink(int fd, ev::loop_ref& loop)
			: loop(loop), fd(fd)
		{}

		void onReceive(size_t size);

		virtual ssize_t send(const msghdr* msg, int flags) = 0;

	public:
		~UdpLink();

		static UdpLink* connect(SocketAddress addr);

		UdpChannel* getChannel(int8_t id, bool reliable);

		boost::signals::connection connectClosed(OnClosed::slot_function_type cb);

		void close();
};

class UdpSocket : public Socket {
	private:
		typedef std::map<SocketAddress, UdpLink*> peers_map;

		int fd;
		ev::io watcher;
		peers_map peers;
		OnAccept onAccept;
		ev::loop_ref& loop;

		void onPacketArrived(ev::io& io, int revents);

		UdpSocket(int fd, ev::loop_ref& loop)
			: fd(fd), watcher(loop), loop(loop)
		{
			watcher.set<UdpSocket, &UdpSocket::onPacketArrived>(this);
			watcher.start(fd, ev::READ);
		}

	public:
		~UdpSocket();

		static UdpSocket* create(SocketAddress addr, ev::loop_ref& loop);

		boost::signals::connection listen(OnAccept::slot_function_type cb);
};

#endif
