#include <stdio.h>
#include "tun.h"
#include <string.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ev.h>

struct tun_device *tun1, *tun2;

void io_cb(struct tun_device *tun)
{
	char buf[0x10000];
	int len = tun_read(tun1, buf, sizeof(buf));
	printf("from: %s (%i)\n", strerror(errno), len);
	len = tun_write(tun2, buf, len);
	printf("to: %s (%i)\n", strerror(errno), len);
}

int main(int argc, char **argv)
{
	tun1 = tun_open("tun-p-%d");
	perror(tun1->name);

	tun2 = tun_open("tun-p-%d");
	perror(tun2->name);

	if (!ev_default_loop(0)) {
		printf("No default loop!\n");
		return 1;
	}

	tun_watcher_set(tun1, ev_default_loop(0), io_cb);
	tun_watcher_start(tun1);

	ev_run(ev_default_loop(0), 0);

	tun_close(tun1);
	tun_close(tun2);

	return 0;
}

