#ifndef NET_NETWORK_H
#define NET_NETWORK_H

#include "exception.hpp"
#include "signal.hpp"
#include "../include/network.hpp"
#include "network-common.hpp"
#include "crypto.hpp"

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
		typedef Signal<void (Link& sender, LinkState oldState)> OnStateChanged;

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
