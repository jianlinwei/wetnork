#include "tunnel-channels.hpp"

Tunnel::Channel::Channel(Stream& next, uint8_t cid)
	: next(next), cid(cid)
{
}

Tunnel::Channel::~Channel()
{
}

ms::connection Tunnel::Channel::connectReceive(OnReceive::slot_function_type cb)
{
	return receive.connect(cb);
}

ms::connection Tunnel::Channel::connectCanSend(OnCanSend::slot_function_type cb)
{
	return canSend.connect(cb);
}
