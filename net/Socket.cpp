#include "network.hpp"

Socket::Socket()
{
}

Socket::~Socket()
{
}

bs2::connection Socket::listen(OnAccept::slot_function_type cb)
{
	if (!accept.empty()) {
		throw InvalidOperation("Already listening");
	}

	return accept.connect(cb);
}
