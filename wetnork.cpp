#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ev.h>

#include "tun.hpp"

class TunDevice *tun1, *tun2;

void io_cb(TunDevice& sender, const Packet& packet)
{
	printf("read: %lu\n", packet.length());
	int len = tun2->write(packet);
	printf("written: %s (%i)\n", strerror(errno), len);
}

int main(int argc, char **argv)
{
	if (!ev_default_loop(0)) {
		printf("No default loop!\n");
		return 1;
	}

	tun1 = TunDevice::create("tun-p-%d", ev_default_loop(0));
	perror(tun1->name().c_str());

	tun2 = TunDevice::create("tun-p-%d", ev_default_loop(0));
	perror(tun2->name().c_str());

	tun1->connectCanRead(io_cb);

	ev_run(ev_default_loop(0), 0);

	delete tun1;
	delete tun2;

	return 0;
}

