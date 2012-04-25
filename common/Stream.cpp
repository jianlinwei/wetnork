#include <network.hpp>

Stream::~Stream()
{
}

void Stream::propagate(const Packet& packet)
{
	read(*this, packet);
}

ms::connection Stream::connectRead(OnRead::slot_function_type fn)
{
	return read.connect(fn);
}
