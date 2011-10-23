#include <stdio.h>
#include "tun.h"
#include "string.h"
#include <sys/socket.h>
#include <linux/if.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char **argv)
{
	char name[IFNAMSIZ];

	int fd1 = open_tun_device("tun-p-%d", name);
	perror(name);

	int fd2 = open_tun_device("tun-p-%d", name);
	perror(name);

	for (;;) {
		char buf[0x10000];
		int len = read(fd1, buf, sizeof(buf));
		printf("from: %s (%i)\n", strerror(errno), len);
		len = write(fd2, buf, len);
		printf("to: %s (%i)\n", strerror(errno), len);
	}

	return 0;
}

