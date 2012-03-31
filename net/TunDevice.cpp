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
	watcher.start();
}
	
const std::string TunDevice::name() const
{
	return _name;
}

ssize_t TunDevice::write(const Packet& packet)
{
	return ::write(fd, packet.data(), packet.length());
}

void TunDevice::watcherEvent(ev::io& io, int revents)
{
	size_t len = 1 << 16;
	std::unique_ptr<uint8_t[]> buffer(new uint8_t[len]);

	len = ::read(fd, buffer.get(), len);

	// TODO: error handling

	propagate(Packet(buffer.release(), 0, len));
}

TunDevice::~TunDevice()
{
	watcher.stop();
	close(fd);
}

