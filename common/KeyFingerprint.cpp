#include <stdexcept>
#include <cstring>

#include <crypto.hpp>
#include <exception.hpp>

KeyFingerprint::KeyFingerprint(const char* data, unsigned int length)
{
	if (!data)
		throw std::invalid_argument("data");
	if (length < 1 || length > 128)
		throw std::invalid_argument("length");

	_data.assign(data, data + length);
}

const char* KeyFingerprint::get() const
{
	return _data.data();
}

const unsigned int KeyFingerprint::length() const
{
	return _data.size();
}

bool KeyFingerprint::operator<(const KeyFingerprint& other) const
{
	return _data < other._data;
}

bool KeyFingerprint::operator==(const KeyFingerprint& other) const
{
	return _data == other._data;
}
