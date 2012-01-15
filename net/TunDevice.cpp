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

ssize_t TunDevice::write(const Packet& packet)
{
	return ::write(fd, packet.data(), packet.length());
}

boost::signals2::connection TunDevice::connectCanRead(TunDevice::OnCanRead::slot_function_type cb)
{
	boost::signals2::connection result = onCanRead.connect(cb);

	if (onCanRead.num_slots() == 1) {
		watcher.start(fd, ev::READ);
	}

	return result;
}

void TunDevice::watcherEvent(ev::io& io, int revents)
{
	if (onCanRead.num_slots() == 0) {
		watcher.stop();
		return;
	}

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

