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
	public:
		typedef Signal<void (Channel& sender, const Packet& packet)> OnReceive;
		typedef Signal<void (Channel& sender)> OnCanSend;

	protected:
		OnReceive receive;
		OnCanSend canSend;

	public:
		virtual ~Channel();

		virtual ssize_t send(const Packet& packet) = 0;

		virtual bs2::connection connectReceive(OnReceive::slot_function_type cb);

		virtual bs2::connection connectCanSend(OnCanSend::slot_function_type cb);
};


class Link {
	public:
		enum class State {
			Invalid,
			Opening,
			Open,
			Closing,
			Closed
		};

		typedef Signal<void (Link& sender, State oldState)> OnStateChanged;

	protected:
		State _state;
		OnStateChanged stateChanged;

		virtual void setState(State state);

	public:
		virtual ~Link();

		virtual State state() const;

		virtual Channel* getChannel(int8_t id, bool reliable) = 0;

		virtual bs2::connection connectStateChanged(OnStateChanged::slot_function_type cb);

		virtual void close() = 0;
};

class Socket {
	public:
		typedef Signal<void (Socket& sender, Link* link)> OnAccept;

	protected:
		OnAccept accept;

		Socket();

	public:
		virtual ~Socket();

		virtual Link* connect(const SocketAddress& peer);

		virtual const SocketAddress& address() const = 0;

		virtual bs2::connection listen(OnAccept::slot_function_type cb);
};

#endif
