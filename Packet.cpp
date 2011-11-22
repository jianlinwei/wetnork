#include "network.hpp"

Packet::Packet(const uint8_t* data, off_t offset, size_t length)
	: _data(data), _offset(offset), _length(length)
{
}

const uint8_t* Packet::data() const
{
	return _data.get() + _offset;
}

size_t Packet::length() const
{
	return _length;
}
