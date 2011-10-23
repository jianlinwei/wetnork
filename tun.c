#include "tun.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

int open_tun_device(const char *name_template, char *name)
{
	struct ifreq ifr;
	int fd, err;

	fd = open("/dev/net/tun", O_RDWR);
	if (fd < 0) {
		return fd;
	}

	memset(&ifr, 0, sizeof(ifr));

	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
	if (name_template) {
		strncpy(ifr.ifr_name, name_template, IFNAMSIZ);
	}

	err = ioctl(fd, TUNSETIFF, (void*) &ifr);
	if (err < 0) {
		close(fd);
		return err;
	}

	if (name) {
		strcpy(name, ifr.ifr_name);
	}
	return fd;
}

