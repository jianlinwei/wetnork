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

void io_cb()
{
	char buf[0x10000];
	int len = tun1->read(buf, sizeof(buf));
	printf("from: %s (%i)\n", strerror(errno), len);
	len = tun2->write(buf, len);
	printf("to: %s (%i)\n", strerror(errno), len);
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

	tun1->connect(io_cb);

	ev_run(ev_default_loop(0), 0);

	delete tun1;
	delete tun2;

	return 0;
}

