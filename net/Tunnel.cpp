#include <stdexcept>
#include <more/make_unique.hpp>
#include <more/bind_this.hpp>

#include <exception.hpp>

#include "tunnel.hpp"
#include "tunnel-channels.hpp"

using namespace std;

Tunnel::Tunnel(ev::loop_ref& loop, CryptoSession&& link, std::unique_ptr<TunDevice>&& tun)
	: link(move(link)), loop(loop), tun(move(tun)),
		dataChannel(new UnreliableChannel(this->link, DataChannelId))
{
	tun->connectRead(more::bind_this(&Tunnel::tunRead, this));
	dataChannel->connectReceive(more::bind_this(&Tunnel::dataRead, this));
}

Tunnel::~Tunnel()
{
	link.close();
}

Tunnel::Channel& Tunnel::getChannel(uint8_t id, bool reliable)
{
	if (reliable) {
	} else {
		switch (id) {
			case 0:
				return *dataChannel;
		}
	}
	throw invalid_argument("id");
}

void Tunnel::propagate(const Packet& packet)
{
	if (packet.length() == 0) {
		throw InvalidOperation("Packet too short");
	}

	uint8_t channel = packet.data()[0];
	bool reliableChannel = !!(channel & 0x80);

	getChannel(channel & ~0x80, reliableChannel).readPacket(packet.skip(1));
}

void Tunnel::tunRead(Stream& sender, const Packet& packet)
{
	dataChannel->writePacket(packet);
}

void Tunnel::dataRead(Channel& sender, const Packet& packet)
{
	tun->write(packet);
}
