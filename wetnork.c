#include <stdio.h>
#include "tun.h"
#include <string.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char **argv)
{
	struct tun_device *tun1, *tun2;

	tun1 = tun_open("tun-p-%d");
	perror(tun1->name);

	tun2 = tun_open("tun-p-%d");
	perror(tun2->name);

	for (;;) {
		char buf[0x10000];
		int len = tun_read(tun1, buf, sizeof(buf));
		printf("from: %s (%i)\n", strerror(errno), len);
		len = tun_write(tun2, buf, len);
		printf("to: %s (%i)\n", strerror(errno), len);
	}

	tun_close(tun1);
	tun_close(tun2);

	return 0;
}

