#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "network.hpp"
#include "network-udp-internal.hpp"

boost::signals::connection UdpChannel::connectReceive(OnReceive::slot_function_type cb)
{
	return onReceive.connect(cb);
}

boost::signals::connection UdpChannel::connectCanSend(OnCanSend::slot_function_type cb)
{
	return onCanSend.connect(cb);
}
