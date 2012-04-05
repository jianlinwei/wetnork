#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>

#include "network.hpp"
#include "network-udp.hpp"

using namespace std;

UdpLink::UdpLink(UdpSocket& parent, int fd, const SocketAddress& peer)
	: peer(peer), parent(parent), fd(fd)
{
	_state = State::Open;
}

UdpLink::~UdpLink()
{
	parent.removeLink(peer);
}

void UdpLink::propagate(const Packet& packet)
{
	if (_state == State::Closed) {
		return;
	}

	Link::propagate(packet);
}

bool UdpLink::write(const Packet& packet)
{
restart:
	int err = sendto(fd, packet.data(), packet.length(), 0, peer.native(), peer.native_len());

	if (err >= 0) {
		return true;
	} else {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return false;
		}
		switch (errno) {
			case EINTR:
				goto restart;

			case EHOSTUNREACH:
			case ENETUNREACH:
			case ENETDOWN:
				// TODO this should raise some event
				return false;

			default:
				throw SocketException(errno, strerror(errno));
		}
	}
}

void UdpLink::close()
{
	setState(State::Closed);
}

