#include "tunnel.hpp"
#include "exception.hpp"
#include <stdexcept>

using namespace std;

Tunnel::Tunnel(ev::loop_ref& loop, Link& link)
	: link(link), loop(loop)
{
}

Tunnel::~Tunnel()
{
	for (channel_map::iterator it = channels.begin(); it != channels.end(); it++) {
		delete it->second;
	}	
}

Tunnel::Channel& Tunnel::getChannel(int8_t id, bool reliable)
{
	if (id < 0) {
		throw invalid_argument("id");
	}

	uint8_t cid = (reliable ? 0x80 : 0) | id;
	if (!channels.count(cid)) {
		if (reliable) {
			channels[cid] = new ReliableChannel(link, cid, loop);
		} else {
			channels[cid] = new UnreliableChannel(link, cid);
		}
	}
	return *channels[cid];
}

void Tunnel::propagate(const Packet& packet)
{
	if (packet.length() == 0) {
		throw InvalidOperation("Packet too short");
	}

	uint8_t channel = packet.data()[0];
	bool reliableChannel = !!(channel & 0x80);

	getChannel(channel & ~0x80, reliableChannel).propagate(packet.skip(1));
}
