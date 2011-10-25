#ifndef TUN_H
#define TUN_H

#include <sys/types.h>

struct tun_device {
	int fd;
	char *name;
};

struct tun_device* tun_open(const char *name_template);
ssize_t tun_read(struct tun_device* tun, char* buffer, size_t len);
ssize_t tun_write(struct tun_device* tun, const char* buffer, size_t len);
void tun_close(struct tun_device *tun);

#endif
