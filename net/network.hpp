#ifndef NET_NETWORK_H
#define NET_NETWORK_H

#include <sys/socket.h>
#include <ev++.h>
#include <netinet/in.h>
#include <map>
#include <cstdint>
#include <string>
#include <arpa/inet.h>
#include <array>
#include <tuple>

#include <exception.hpp>
#include <network.hpp>
#include <SocketAddress.hpp>
#include <signal.hpp>
#include <crypto.hpp>

class SocketException : public Exception {
	private:
		int _number;

	public:
		SocketException(int number, const std::string& what);

		int number() const;
};



class Link : public Stream {
	public:
		enum class State {
			Invalid,
			Opening,
			Open,
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

		virtual ms::connection connectStateChanged(OnStateChanged::slot_function_type cb);

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

		virtual Link& connect(const SocketAddress& peer) = 0;

		virtual const SocketAddress& address() const = 0;

		virtual ms::connection listen(OnAccept::slot_function_type cb);
};

#endif
