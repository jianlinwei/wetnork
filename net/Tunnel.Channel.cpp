#include "tunnel.hpp"

Tunnel::Channel::~Channel()
{
}

bs2::connection Tunnel::Channel::connectReceive(OnReceive::slot_function_type cb)
{
	return receive.connect(cb);
}

bs2::connection Tunnel::Channel::connectCanSend(OnCanSend::slot_function_type cb)
{
	return canSend.connect(cb);
}
