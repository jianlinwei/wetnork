#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "network.hpp"
#include "network-udp.hpp"

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

	peers_map localPeers(std::move(peers));
	localPeers.clear();
}

void UdpSocket::removeLink(const SocketAddress& link)
{
	if (!peers.empty()) {
		peers.erase(link);
	}
}

void UdpSocket::onPacketArrived(ev::io& io, int revents)
{
	sockaddr_storage addr;
	socklen_t addrlen = sizeof(addr);

	// look at the error queue, too

	uint8_t buffer[1 << 16];

	ssize_t plen = recvfrom(fd, buffer, sizeof(buffer), 0, reinterpret_cast<sockaddr*>(&addr), &addrlen);
	if (plen < 0) {
		// no error will happen here. trust me
		return;
	}

	std::unique_ptr<uint8_t[]> packetBuffer(new uint8_t[plen]);

	memcpy(packetBuffer.get(), buffer, plen);

	SocketAddress s_addr(reinterpret_cast<sockaddr*>(&addr));

	if (peers.count(s_addr) == 0) {
		if (!accept.empty()) {
			// TODO: accept connections
		}
	} else {
		UdpLink& link = *peers.at(s_addr);

		link.propagate(Packet(packetBuffer.release(), 0, plen));
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

UdpLink& UdpSocket::connect(const SocketAddress& peer)
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

	peers[peer] = std::unique_ptr<UdpLink>(new UdpLink(*this, fd, peer));
	
	return *peers.at(peer);
	
	// TODO: connect handling
}

const SocketAddress& UdpSocket::address() const
{
	return _address;
}

