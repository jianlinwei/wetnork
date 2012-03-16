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

UdpLink::UdpLink(UdpSocket& parent, int fd, const SocketAddress& peer, ev::loop_ref& loop)
	: loop(loop), peer(peer), parent(parent), fd(fd)
{
	_state = State::Open;
}

UdpLink::~UdpLink()
{
	if (_state == State::Opening || _state == State::Open) {
		close();
	}
	parent.removeLink(peer);
}

void UdpLink::propagatePacket(const Packet& packet)
{
	if (packet.length() == 0) {
		throw InvalidOperation("Packet too short");
	}

	receive(*this, packet);
}

ssize_t UdpLink::send(const msghdr* msg)
{
	SocketAddress peer = this->peer;

	msghdr actual = *msg;
	actual.msg_name = const_cast<sockaddr*>(peer.native());
	actual.msg_namelen = peer.native_len();

	return sendmsg(fd, &actual, MSG_DONTWAIT);
}

void UdpLink::close()
{
	setState(State::Closed);
}

