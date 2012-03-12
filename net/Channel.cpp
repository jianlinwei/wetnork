#include "network.hpp"

Channel::~Channel()
{
}

bs2::connection Channel::connectReceive(OnReceive::slot_function_type cb)
{
	return receive.connect(cb);
}

bs2::connection Channel::connectCanSend(OnCanSend::slot_function_type cb)
{
	return canSend.connect(cb);
}
