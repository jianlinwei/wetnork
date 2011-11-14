#ifndef TUN_H
#define TUN_H

#include <sys/types.h>
#include <ev.h>

struct tun_device {
	int fd;
	char *name;
	void *data;

	// private stuffs
	ev_loop *loop;
	ev_io io_watcher;
	void (*io_cb)(struct tun_device *tun);
	int watcher_running;
};

struct tun_device *tun_open(const char *name_template);

ssize_t tun_read(struct tun_device *tun, char *buffer, size_t len);
ssize_t tun_write(struct tun_device *tun, const char *buffer, size_t len);

void tun_watcher_set(struct tun_device *tun, ev_loop *loop, void (*cb)(struct tun_device *tun));
void tun_watcher_start(struct tun_device *tun);
void tun_watcher_stop(struct tun_device *tun);

void tun_close(struct tun_device *tun);

#endif
