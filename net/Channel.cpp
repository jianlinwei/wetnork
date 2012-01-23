#include "network.hpp"

Channel::~Channel()
{
}

SignalConnection Channel::connectReceive(OnReceive::slot_function_type cb)
{
	if (!receive.empty()) {
		throw InvalidOperation("Receive already connected");
	}

	return receive.connect(cb);
}

SignalConnection Channel::connectCanSend(OnCanSend::slot_function_type cb)
{
	if (!canSend.empty()) {
		throw InvalidOperation("CanSend already connected");
	}
	
	return canSend.connect(cb);
}
