#ifndef NETWORK_H
#define NETWORK_H

#include <sys/socket.h>
#include <ev++.h>
#include <netinet/in.h>
#include <boost/signals2.hpp>
#include <map>
#include <stdint.h>
#include <string>
#include <arpa/inet.h>
#include <boost/shared_array.hpp>
#include "exception.hpp"

class NetworkException : public Exception {
	public:
		NetworkException(const char* what);
};

class BadAddress : public NetworkException {
	public:
		BadAddress(const char* what);
};

class BadSend : public NetworkException {
	public:
		BadSend(const char* what);
};

struct SocketAddress {
	private:
		union {
			sockaddr_in in;
			sockaddr_in6 in6;
		} addr;
		socklen_t len;

		SocketAddress();

	public:
		SocketAddress(const sockaddr* addr);						

		static SocketAddress parse(const std::string& addr, in_port_t port);

		int family() const;

		const sockaddr* native() const;

		socklen_t native_len() const;

		bool operator<(const SocketAddress& other) const;

		bool operator==(const SocketAddress& other) const;
};





class Packet {
	private:
		boost::shared_array<const uint8_t> _data;
		off_t _offset;
		size_t _length;

		Packet(const boost::shared_array<const uint8_t>& data, off_t offset, size_t length);

	public:
		Packet(uint8_t* data, off_t offset, size_t length);

		const uint8_t* data() const;

		size_t length() const;

		Packet skip(size_t bytes) const;
};

class Channel {
	protected:
		typedef boost::signals2::signal<void (Channel& sender, const Packet& packet)> OnReceive;
		typedef boost::signals2::signal<void (Channel& sender)> OnCanSend;

	public:
		virtual ~Channel();

		virtual ssize_t send(const Packet& packet) = 0;

		virtual boost::signals2::connection connectReceive(OnReceive::slot_function_type cb) = 0;

		virtual boost::signals2::connection connectCanSend(OnCanSend::slot_function_type cb) = 0;
};

class Link {
	protected:
		typedef boost::signals2::signal<void (Link& sender)> OnClosed;

	public:
		virtual ~Link();

		virtual Channel* getChannel(int8_t id, bool reliable) = 0;

		virtual boost::signals2::connection connectClosed(OnClosed::slot_function_type cb) = 0;

		virtual void close() = 0;
};

class Socket {
	protected:
		typedef boost::signals2::signal<void (Socket& sender, Link* link)> OnAccept;

		Socket();

	public:
		virtual ~Socket();

		virtual boost::signals2::connection listen(OnAccept::slot_function_type cb) = 0;
};

#endif
