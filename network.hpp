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
		const off_t _offset;
		const size_t _length;

	public:
		Packet(const uint8_t* data, off_t offset, size_t length)
			: _data(data), _offset(offset), _length(length)
		{}

		~Packet()
		{
		}

		const uint8_t* data() const { return _data.get() + _offset; }

		size_t length() const { return _length; }
};

class Channel {
	protected:
		typedef boost::signal<void (Packet packet)> OnReceive;

	public:
		virtual ssize_t send(const char* buffer, size_t len) = 0;

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
	private:
		OnReceive onReceive;

		void propagatePacket(Packet packet)
		{
			onReceive(packet);
		}

	public:
		ssize_t send(const char* buffer, size_t len);

		boost::signals::connection connectReceive(OnReceive::slot_function_type cb);
};

class UdpLink : public Link {
	friend class UdpSocket;
	private:
		class SocketHandler {
			protected:
				int fd;
				UdpLink& parent;

				SocketHandler(int fd, UdpLink& parent)
					: fd(fd), parent(parent)
				{}

			public:
				void onReceive(size_t size);
		};
		class StandaloneHandler : public SocketHandler {
			private:
				ev::io watcher;

				void onPacketArrived(ev::io& io, int revents);

			public:
				StandaloneHandler(int fd, UdpLink& parent, ev_loop* loop)
					: SocketHandler(fd, parent), watcher(loop)
				{
					watcher.set<StandaloneHandler, &StandaloneHandler::onPacketArrived>(this);
					watcher.start(fd, ev::READ);
				}
		};

		typedef std::map<uint8_t, UdpChannel*> channel_map;

		OnClosed onClosed;
		SocketHandler* handler;
		channel_map channels;

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

		void onPacketArrived(ev::io& io, int revents);

		UdpSocket(int fd, ev_loop* loop)
			: fd(fd), watcher(loop)
		{
			watcher.set<UdpSocket, &UdpSocket::onPacketArrived>(this);
			watcher.start(fd, ev::READ);
		}

	public:
		~UdpSocket();

		static UdpSocket* create(SocketAddress addr, ev_loop* loop);

		boost::signals::connection listen(OnAccept::slot_function_type cb);
};

#endif
