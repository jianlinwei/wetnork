#ifndef NET_NETWORK_H
#define NET_NETWORK_H

#include "exception.hpp"
#include "signal.hpp"
#include "../include/network.hpp"

#include <sys/socket.h>
#include <ev++.h>
#include <netinet/in.h>
#include <map>
#include <stdint.h>
#include <string>
#include <arpa/inet.h>
#include <boost/shared_array.hpp>

class SocketException : public Exception {
	private:
		int _number;

	public:
		SocketException(int number, const std::string& what);

		int number() const;
};

class NetworkException : public Exception {
	public:
		explicit NetworkException(const std::string& what);
};

class BadAddress : public NetworkException {
	public:
		explicit BadAddress(const std::string& what);
};

class BadSend : public NetworkException {
	public:
		explicit BadSend(const std::string& what);
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
		typedef Signal<void (Channel& sender, const Packet& packet)> OnReceive;
		typedef Signal<void (Channel& sender)> OnCanSend;

		OnReceive receive;
		OnCanSend canSend;

	public:
		virtual ~Channel();

		virtual ssize_t send(const Packet& packet) = 0;

		virtual SignalConnection connectReceive(OnReceive::slot_function_type cb);

		virtual SignalConnection connectCanSend(OnCanSend::slot_function_type cb);
};

enum class LinkState {
	Invalid,
	Opening,
	Open,
	Closing,
	Closed
};

class Link {
	protected:
		typedef Signal<void (Link& sender, LinkState state)> OnStateChanged;

		LinkState _state;
		OnStateChanged stateChanged;

		virtual void setState(LinkState state);

	public:
		virtual ~Link();

		virtual LinkState state() const;

		virtual Channel* getChannel(int8_t id, bool reliable) = 0;

		virtual SignalConnection connectStateChanged(OnStateChanged::slot_function_type cb);

		virtual void close() = 0;
};

class Socket {
	protected:
		typedef Signal<void (Socket& sender, Link* link)> OnAccept;

		OnAccept accept;

		Socket();

	public:
		virtual ~Socket();

		virtual Link* connect(const SocketAddress& peer);

		virtual const SocketAddress& address() const = 0;

		virtual SignalConnection listen(OnAccept::slot_function_type cb);
};

#endif
