#ifndef NET_NETWORK_COMMON_H
#define NET_NETWORK_COMMON_H

#include <sys/socket.h>
#include <ev++.h>
#include <netinet/in.h>
#include <map>
#include <stdint.h>
#include <string>
#include <arpa/inet.h>
#include <boost/shared_array.hpp>

#include <network.hpp>
#include <exception.hpp>
#include <signal.hpp>

class Packet {
	private:
		struct NullDeleter {
			void operator()(const uint8_t*) {}
		};

		boost::shared_array<const uint8_t> _data;
		ptrdiff_t _offset;
		size_t _length;

		Packet(const boost::shared_array<const uint8_t>& data, ptrdiff_t offset, size_t length);

	public:
		Packet(uint8_t* data, ptrdiff_t offset, size_t length, bool capture = true);

		const uint8_t* data() const;

		size_t length() const;

		Packet skip(size_t bytes) const;
};

#endif
