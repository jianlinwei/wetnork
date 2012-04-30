#include <Packet.hpp>

Packet::Packet(const std::shared_ptr<const uint8_t>& data, ptrdiff_t offset, size_t length)
	: _data(data), _offset(offset), _length(length)
{
}

Packet::Packet(const uint8_t* data, ptrdiff_t offset, size_t length)
	: _data(data, ArrayDelete), _offset(offset), _length(length)
{
}

Packet::Packet(const uint8_t* data, ptrdiff_t offset, size_t length, nocapture_t)
	: _data(data, NullDelete), _offset(offset), _length(length)
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

Packet Packet::skip(size_t bytes) const
{
	if (bytes > _length) {
		bytes = _length;
	}

	return Packet(_data, _offset + bytes, _length - bytes);
}



void Packet::NullDelete(const uint8_t*)
{
}

void Packet::ArrayDelete(const uint8_t* ptr)
{
	delete[] ptr;
}
