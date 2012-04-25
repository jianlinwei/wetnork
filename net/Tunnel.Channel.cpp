#include "tunnel-channels.hpp"

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
