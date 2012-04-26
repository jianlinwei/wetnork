#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdexcept>
#include <unistd.h>
#include <more/make_unique.hpp>

#include "TunRegistry.hpp"

using namespace host;

const TunDevice& TunRegistry::createDevice(const std::string& nameTemplate)
{
	struct ifreq ifr;

	int fd = open(tunCtl_.c_str(), O_RDWR);
	if (fd < 0) {
		throw FileNotFound("Could not open " + tunCtl_);
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
	std::strncpy(ifr.ifr_name, nameTemplate.c_str(), IFNAMSIZ);

	int err = ioctl(fd, TUNSETIFF, reinterpret_cast<void*>(&ifr));
	if (err < 0) {
		close(fd);
		throw InvalidOperation(std::string("Could not ioctl() tun: ") + strerror(errno));
	}

	std::string ifName = ifr.ifr_name;
	uint32_t ifIndex = if_nametoindex(ifr.ifr_name);

	devices[ifName] = std::unique_ptr<TunDevice>(new TunDevice(fd, ifIndex, ifName));
	return *devices[ifName];
}

const TunDevice* TunRegistry::findDevice(const std::string& name) const
{
	auto pos = devices.find(name);
	if (pos == devices.end()) {
		return nullptr;
	} else {
		return pos->second.get();
	}
}

bool TunRegistry::closeDevice(const std::string& name)
{
	return devices.erase(name);
}
