#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "network.hpp"
#include "network-udp-internal.hpp"

UdpChannel::UdpChannel(UdpLink& parent, uint8_t cid)
	: parent(parent), cid(cid)
{
}

SignalConnection UdpChannel::connectReceive(OnReceive::slot_function_type cb)
{
	return onReceive.connect(cb);
}

SignalConnection UdpChannel::connectCanSend(OnCanSend::slot_function_type cb)
{
	return onCanSend.connect(cb);
}

