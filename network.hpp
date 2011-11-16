#ifndef NETWORK_H
#define NETWORK_H

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
class bad_packet {};

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

		bool operator<(const SocketAddress& other) const;

		bool operator==(const SocketAddress& other) const;
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
		typedef boost::signal<void ()> OnCanSend;

	public:
		virtual ssize_t send(const uint8_t* buffer, size_t len) = 0;

		virtual boost::signals::connection connectReceive(OnReceive::slot_function_type cb) = 0;

		virtual boost::signals::connection connectCanSend(OnCanSend::slot_function_type cb) = 0;
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

#endif
