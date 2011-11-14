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

struct tun_device *tun_open(const char *name_template)
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
		close(fd);
		return NULL;
	}

	result = (struct tun_device*) malloc(sizeof(struct tun_device));
	if (result == NULL) {
		close(fd);
		free(tun_name);
		return NULL;
	}

	result->name = tun_name;
	result->fd = fd;
	return result;
}

ssize_t tun_read(struct tun_device *tun, char *buffer, size_t len)
{
	return read(tun->fd, buffer, len);
}

ssize_t tun_write(struct tun_device *tun, const char *buffer, size_t len)
{
	return write(tun->fd, buffer, len);
}

static void tun_io_cb(ev_loop *loop, ev_io *io, int revents)
{
	struct tun_device* tun = (struct tun_device*) io->data;

	tun->io_cb(tun);
}

void tun_watcher_set(struct tun_device *tun, ev_loop *loop, void (*cb)(struct tun_device *tun))
{
	ev_io_init(&tun->io_watcher, tun_io_cb, tun->fd, EV_READ);
	tun->io_watcher.data = tun;
	tun->loop = loop;
	tun->io_cb = cb;
}

void tun_watcher_start(struct tun_device *tun)
{
	ev_io_start(tun->loop, &tun->io_watcher);
}

void tun_watcher_stop(struct tun_device *tun)
{
	ev_io_stop(tun->loop, &tun->io_watcher);
}

void tun_close(struct tun_device *tun)
{
	if (tun->watcher_running) {
		tun_watcher_stop(tun);
	}
	close(tun->fd);
	free(tun->name);
	free(tun);
}
