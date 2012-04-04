#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
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
	int err = sendto(fd, packet.data(), packet.length(), 0, peer.native(), peer.native_len());

	if (err > 0) {
		return true;
	} else if (err == EAGAIN || err == EWOULDBLOCK) {
		return false;
	} else {
		throw SocketException(errno, strerror(errno));
	}
}

void UdpLink::close()
{
	setState(State::Closed);
}

