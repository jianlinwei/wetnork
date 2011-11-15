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




class bad_address {};

struct SocketAddress {
	private:
		union {
			sockaddr_in in;
			sockaddr_in6 in6;
		} addr;
		socklen_t len;

		SocketAddress()
		{
		}

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





class Channel {
	protected:
		typedef boost::signal<void ()> OnCanReceive;
		typedef boost::signal<void ()> OnCanSend;

	public:
		virtual ssize_t receive(char* buffer, size_t len) = 0;
		virtual ssize_t send(const char* buffer, size_t len) = 0;

		virtual boost::signals::connection connectReceive(OnCanReceive::slot_function_type cb) = 0;
		virtual boost::signals::connection connectSend(OnCanSend::slot_function_type cb) = 0;
};

class Link {
	protected:
		typedef boost::signal<void ()> OnClosed;

	public:
		virtual Channel* getChannel(int8_t id, bool reliable) = 0;

		virtual void connectClosed(OnClosed::slot_function_type cb) = 0;

		virtual void close() = 0;
};

class Socket {
	protected:
		typedef boost::signal<void (Link*)> OnAccept;

		Socket()
		{
		}

	public:
		virtual boost::signals::connection listen(OnAccept::slot_function_type cb) = 0;
};




class UdpChannel;
class UdpLink;
class UdpSocket;

class UdpChannel : public Channel {
	private:
		OnCanReceive onCanReceive;
		OnCanSend onCanSend;

	public:
		ssize_t receive(char* buffer, size_t len);
		ssize_t send(const char* buffer, size_t len);

		boost::signals::connection connectReceive(OnCanReceive::slot_function_type cb);
		boost::signals::connection connectSend(OnCanSend::slot_function_type cb);
};

class UdpLink : public Link {
	private:
		OnClosed onClosed;

	public:
		static UdpLink* connect(SocketAddress addr);

		Channel* getChannel(int8_t id, bool reliable) = 0;

		void connectClosed(OnClosed::slot_function_type cb) = 0;

		void close() = 0;
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
