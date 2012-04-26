#include <host/TunDevice.hpp>

#include <unistd.h>

using namespace host;

TunDevice::TunDevice(int fd, int ifIndex, const std::string& name)
	: fd_(fd), ifIndex_(ifIndex), name_(name)
{
}

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

void TunDevice::close()
{
	if (fd_ >= 0) {
		::close(fd_);
	}
}

int TunDevice::fd() const
{
	return fd_;
}

int TunDevice::ifIndex() const
{
	return ifIndex_;
}

const std::string& TunDevice::name() const
{
	return name_;
}
