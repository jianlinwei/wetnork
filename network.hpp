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
		virtual ~Socket() = 0;

		virtual boost::signals::connection listen(OnAccept::slot_function_type cb) = 0;
};




class UdpLink;

class UdpSocket : public Socket {
	private:
		int fd;
		ev::io watcher;
		std::map<SocketAddress, UdpLink*> peers;

		void onPacketArrived(ev::io& io, int revents);

		UdpSocket(int fd, ev_loop* loop)
			: fd(fd), watcher(loop)
		{
			watcher.set<UdpSocket, &UdpSocket::onPacketArrived>(this);
		}

	public:
		static UdpSocket* create(struct sockaddr* addr, socklen_t len);

		boost::signals::connection listen(OnAccept::slot_function_type cb);
};

class UdpChannel : public Channel {
	private:
		OnCanReceive onCanReceive;
		OnCanSend onCanSend;

	public:
		static UdpChannel* connect(struct sockaddr* addr, socklen_t len);

		ssize_t receive(char* buffer, size_t len);
		ssize_t send(const char* buffer, size_t len);

		boost::signals::connection connectReceive(OnCanReceive::slot_function_type cb);
		boost::signals::connection connectSend(OnCanSend::slot_function_type cb);
};

class UdpLink : public Link {
	private:
		OnClosed onClosed;

	public:
		Channel* getChannel(int8_t id, bool reliable);

		void connectClosed(OnClosed::slot_function_type cb);

		void close();
};

#endif
