#include "network.hpp"

Packet::Packet(const boost::shared_array<const uint8_t>& data, ptrdiff_t offset, size_t length)
	: _data(data), _offset(offset), _length(length)
{
}

Packet::Packet(uint8_t* data, ptrdiff_t offset, size_t length, bool capture)
	: _offset(offset), _length(length)
{
	if (capture) {
		_data = boost::shared_array<const uint8_t>(data);
	} else {
		_data = boost::shared_array<const uint8_t>(data, NullDeleter());
	}
}

const uint8_t* Packet::data() const
{
	return _data.get() + _offset;
}

size_t Packet::length() const
{
	return _length;
}

Packet Packet::skip(size_t bytes) const
{
	if (bytes > _length) {
		bytes = _length;
	}

	return Packet(_data, _offset + bytes, _length - bytes);
}
