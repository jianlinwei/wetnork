#include <host/TunDevice.hpp>

#include <cstring>
#include <unistd.h>

#include <exception.hpp>

using namespace host;

TunDevice::TunDevice(TunDevice&& other)
	: fd_(other.fd_), ifIndex_(other.ifIndex_), name_(std::move(other.name_))
{
	other.fd_ = -1;
	other.ifIndex_ = -1;
}

TunDevice& TunDevice::operator=(TunDevice&& rhs)
{
	close();

	fd_ = rhs.fd_;
	name_ = std::move(rhs.name_);

	rhs.fd_ = -1;
	rhs.ifIndex_ = -1;

	return *this;
}

TunDevice::~TunDevice()
{
	close();
}

TunDevice TunDevice::dup() const
{
	int fd = ::dup(fd_);
	if (fd < 0) {
		throw InvalidOperation(std::string("Could not duplicate file descriptor: ") + std::strerror(errno));
	}

	return TunDevice(fd, ifIndex_, name_);
}

void TunDevice::close()
{
	if (fd_ >= 0) {
		::close(fd_);
	}
}
