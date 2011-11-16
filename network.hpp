#ifndef LINK_H
#define LINK_H

#include <sys/socket.h>
#include <ev++.h>
#include <netinet/in.h>
#include <boost/signal.hpp>
#include <map>
#include <queue>
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

		size_t length() const { return _length - _offset; }
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
	private:
		class ChannelHandler {
			protected:
				UdpChannel& parent;

			public:
				ChannelHandler(UdpChannel& parent)
					: parent(parent)
				{}

				virtual ssize_t send(const uint8_t* buffer, size_t len) = 0;
				virtual void propagate(Packet packet) = 0;
		};
		class UnreliableChannelHandler : public ChannelHandler {
			public:
				UnreliableChannelHandler(UdpChannel& parent)
					: ChannelHandler(parent)
				{}

				ssize_t send(const uint8_t* buffer, size_t len);
				void propagate(Packet packet);
		};
		class ReliableChannelHandler : public ChannelHandler {
			private:
				ev::timer timeout;
				std::queue<Packet> queue;
				uint32_t currentSeqNum;

				void onTimeout(ev::timer& timer, int revents);

			public:
				ReliableChannelHandler(UdpChannel& parent, ev::loop_ref& loop)
					: ChannelHandler(parent), timeout(loop)
				{}

				ssize_t send(const uint8_t* buffer, size_t len);
				void propagate(Packet packet);
		};

		OnReceive onReceive;
		UdpLink& parent;
		uint8_t cid;
		ChannelHandler* handler;

		void propagatePacket(Packet packet)
		{
			onReceive(packet);
		}

	protected:
		UdpChannel(UdpLink& parent, int8_t id, bool reliable, ev::loop_ref& loop)
			: parent(parent)
		{
			if (reliable) {
				cid = 0x80 | id;
				handler = new ReliableChannelHandler(*this, loop);
			} else {
				cid = id;
				handler = new UnreliableChannelHandler(*this);
			}
		}

	public:
		~UdpChannel()
		{
			delete handler;
		}

		ssize_t send(const uint8_t* buffer, size_t len);

		boost::signals::connection connectReceive(OnReceive::slot_function_type cb);
};

class UdpLink : public Link {
	friend class UdpChannel;
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
				virtual void onReceive(size_t size);
				virtual ssize_t send(const msghdr* msg, int flags) = 0;
		};
		class AcceptedHandler : public SocketHandler {
			private:
				SocketAddress peer;

			protected:
				AcceptedHandler(int fd, UdpLink& parent, SocketAddress peer)
					: SocketHandler(fd, parent), peer(peer)
				{}

			public:
				ssize_t send(const msghdr* msg, int flags);
		};
		class ConnectedHandler : public SocketHandler {
			private:
				ev::io watcher;

				void onPacketArrived(ev::io& io, int revents);

			protected:
				ConnectedHandler(int fd, UdpLink& parent, ev::loop_ref& loop)
					: SocketHandler(fd, parent), watcher(loop)
				{
					watcher.set<ConnectedHandler, &ConnectedHandler::onPacketArrived>(this);
					watcher.start(fd, ev::READ);
				}

			public:
				ssize_t send(const msghdr* msg, int flags);
		};

		typedef std::map<uint8_t, UdpChannel*> channel_map;

		OnClosed onClosed;
		SocketHandler* handler;
		channel_map channels;
		ev::loop_ref& loop;

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
