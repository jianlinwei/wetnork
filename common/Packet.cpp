#include <Packet.hpp>

Packet Packet::skip(size_t bytes) const
{
	if (bytes > length_) {
		bytes = length_;
	}

	return Packet(data_, offset_ + bytes, length_ - bytes);
}



void Packet::NullDelete(const uint8_t*)
{
}

void Packet::ArrayDelete(const uint8_t* ptr)
{
	delete[] ptr;
}
