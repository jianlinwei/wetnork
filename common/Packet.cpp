#include <Packet.hpp>

Packet Packet::skip(size_t bytes) const
{
	if (bytes > length_) {
		bytes = length_;
	}

	return Packet(data_, offset_ + bytes, length_ - bytes);
}

Packet Packet::truncate(size_t length) const
{
	if (length > length_) {
		length = length_;
	}

	return Packet(data_, offset_, length);
}



void Packet::NullDelete(const uint8_t*)
{
}

void Packet::ArrayDelete(const uint8_t* ptr)
{
	delete[] ptr;
}
