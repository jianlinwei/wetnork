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



class Link {
	public:
		enum class State {
			Invalid,
			Opening,
			Open,
			Closed
		};

		typedef Signal<void (Link& sender, State oldState)> OnStateChanged;
		typedef Signal<void (Link& sender, const Packet& packet)> OnReceive;

	protected:
		State _state;
		OnStateChanged stateChanged;
		OnReceive receive;

		virtual void setState(State state);

	public:
		virtual ~Link();

		virtual State state() const;

		virtual bs2::connection connectStateChanged(OnStateChanged::slot_function_type cb);

		virtual bs2::connection connectReceive(OnReceive::slot_function_type cb);

		virtual ssize_t send(const msghdr* msg) = 0;

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
