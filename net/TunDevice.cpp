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

#include "tun.hpp"

TunDevice::TunDevice(const int fd, const std::string& name, const ev::loop_ref& loop)
	: fd(fd), _name(name), watcher(loop)
{
	watcher.set<TunDevice, &TunDevice::watcherEvent>(this);
}
	
const std::string TunDevice::name() const
{
	return _name;
}

TunDevice* TunDevice::create(const std::string& name_template, const ev::loop_ref& loop)
{
	struct ifreq ifr;
	int fd;
	int err;

	fd = open("/dev/net/tun", O_RDWR);
	if (fd < 0) {
		return NULL;
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
	if (name_template.size()) {
		strncpy(ifr.ifr_name, name_template.c_str(), IFNAMSIZ);
	}

	err = ioctl(fd, TUNSETIFF, reinterpret_cast<void*>(&ifr));
	if (err < 0) {
		close(fd);
		return NULL;
	}

	return new TunDevice(fd, std::string(ifr.ifr_name), loop);
}

ssize_t TunDevice::write(const Packet& packet)
{
	return ::write(fd, packet.data(), packet.length());
}

boost::signals2::connection TunDevice::connect(TunDevice::OnCanRead::slot_function_type cb)
{
	boost::signals2::connection result = onCanRead.connect(cb);

	if (onCanRead.num_slots() == 1) {
		watcher.start(fd, ev::READ);
	}

	return result;
}

void TunDevice::watcherEvent(ev::io& io, int revents)
{
	size_t len = 65536;
	uint8_t* buffer = new uint8_t[len];

	len = ::read(fd, buffer, len);

	// TODO: error handling

	onCanRead(*this, Packet(buffer, 0, len));
}

TunDevice::~TunDevice()
{
	if (!onCanRead.empty()) {
		watcher.stop();
	}
	close(fd);
}

