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

