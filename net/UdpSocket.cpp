#include "network.hpp"
#include "network-udp-internal.hpp"

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

// *sigh* ev++ monopolizes 'set' ...
using std::bad_alloc;
using std::invalid_argument;
using std::nothrow;
using std::string;

UdpSocket::UdpSocket(int fd, const SocketAddress& address, ev::loop_ref& loop)
	: fd(fd), watcher(loop), loop(loop), _address(address)
{
	watcher.set<UdpSocket, &UdpSocket::onPacketArrived>(this);
	watcher.start(fd, ev::READ);
}

UdpSocket::~UdpSocket()
{
	watcher.stop();

	for (peers_map::iterator it = peers.begin(); it != peers.end(); it++) {
		delete it->second;
	}
}

void UdpSocket::onPacketArrived(ev::io& io, int revents)
{
	sockaddr_storage addr;
	socklen_t addrlen = sizeof(addr);

	// look at the error queue, too

	ssize_t plen = recvfrom(fd, nullptr, 0, MSG_PEEK | MSG_TRUNC,
			reinterpret_cast<sockaddr*>(&addr), &addrlen);
	if (plen < 0) {
		// no error will happen here. trust me
		return;
	}

	uint8_t* packetBuffer = new (nothrow) uint8_t[plen];
	if (!packetBuffer) {
		throw bad_alloc();
	}

	recv(fd, packetBuffer, plen, 0);

	SocketAddress s_addr(reinterpret_cast<sockaddr*>(&addr));

	if (peers.count(s_addr) == 0) {
		if (!accept.empty()) {
			// TODO: accept connections
		}
	} else {
		UdpLink* link = peers[s_addr];

		link->propagatePacket(Packet(packetBuffer, 0, plen));
	}
}

UdpSocket* UdpSocket::create(const SocketAddress& addr, ev::loop_ref& loop)
{
	int fd = socket(addr.family(), SOCK_DGRAM, IPPROTO_UDP);

	if (fd < 0) {
		throw SocketException(errno, string("Could not open socket: ") + strerror(errno));
	}

	int err = bind(fd, addr.native(), addr.native_len());
	if (err < 0) {
		close(fd);
		throw SocketException(errno, string("Could not bind: ") + strerror(errno));
	}

	return new UdpSocket(fd, addr, loop);
}

UdpLink* UdpSocket::connect(const SocketAddress& peer)
{
	if (peer.family() != _address.family()) {
		throw invalid_argument("peer");
	}

	int fd = ::socket(peer.family(), SOCK_DGRAM, IPPROTO_UDP);
	if (fd < 0) {
		throw SocketException(errno, string("Could not open socket: ") + strerror(errno));
	}

	int err = ::connect(fd, peer.native(), peer.native_len());
	if (err < 0) {
		throw SocketException(errno, string("Could not connect: ") + strerror(errno));
	}

	close(fd);

	peers[peer] = new UdpLink(fd, peer, loop);
	
	return peers[peer];
	
	// TODO: connect handling
}

const SocketAddress& UdpSocket::address() const
{
	return _address;
}

