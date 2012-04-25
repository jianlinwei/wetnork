#include "network.hpp"

Socket::Socket()
{
}

Socket::~Socket()
{
}

ms::connection Socket::listen(OnAccept::slot_function_type cb)
{
	return accept.connect(cb);
}
