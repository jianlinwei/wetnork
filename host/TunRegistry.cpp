#include "tun.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdexcept>

using namespace std;

int TunRegistry::findDevice(const string& name) const
{
	if (!devices.count(name)) {
		return -1;
	}
	return devices.at(name);
}

string TunRegistry::createDevice(const string& nameTemplate)
{
	struct ifreq ifr;
	int fd;
	int err;
	string result;

	fd = open("/dev/net/tun", O_RDWR);
	if (fd < 0) {
		throw FileNotFound("Could not open /dev/net/tun");
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
	if (nameTemplate.size()) {
		strncpy(ifr.ifr_name, nameTemplate.c_str(), IFNAMSIZ);
	}

	err = ioctl(fd, TUNSETIFF, reinterpret_cast<void*>(&ifr));
	if (err < 0) {
		close(fd);
		throw InvalidOperation(string("Could not ioctl() tun: ") + strerror(errno));
	}

	result = ifr.ifr_name;
	devices[result] = fd;
	return result;
}

void TunRegistry::closeDevice(const string& name)
{
	close(findDevice(name));
	devices.erase(name);
}
