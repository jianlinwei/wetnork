#include "tun.h"

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

struct tun_device* tun_open(const char *name_template)
{
	struct ifreq ifr;
	int fd, err;
	struct tun_device *result;
	char *tun_name;

	fd = open("/dev/net/tun", O_RDWR);
	if (fd < 0) {
		return NULL;
	}

	memset(&ifr, 0, sizeof(ifr));

	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
	if (name_template) {
		strncpy(ifr.ifr_name, name_template, IFNAMSIZ);
	}

	err = ioctl(fd, TUNSETIFF, (void*) &ifr);
	if (err < 0) {
		close(fd);
		return NULL;
	}

	tun_name = strndup(ifr.ifr_name, IFNAMSIZ);
	if (tun_name == NULL) {
		return NULL;
	}

	result = (struct tun_device*) malloc(sizeof(struct tun_device));
	if (result == NULL) {
		return NULL;
	}

	result->name = tun_name;
	result->fd = fd;
	return result;
}

void tun_close(struct tun_device *tun)
{
	close(tun->fd);
	free(tun->name);
	free(tun);
}
