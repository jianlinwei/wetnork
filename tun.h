#ifndef TUN_H
#define TUN_H

struct tun_device {
	int fd;
	char *name;
};

struct tun_device* tun_open(const char *name_template);
void tun_close(struct tun_device *tun);

#endif
