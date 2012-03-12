#include "crypto.hpp"

#include "exception.hpp"
#include <stdexcept>
#include <string.h>

KeyFingerprint::KeyFingerprint(const char* data, int length)
{
	set(data, length);
}

KeyFingerprint::KeyFingerprint(const KeyFingerprint& fp)
	: KeyFingerprint(fp.get(), fp.length())
{
}

KeyFingerprint::~KeyFingerprint()
{
	delete[] _data;
}

void KeyFingerprint::set(const char* data, int length)
{
	if (!data)
		throw std::invalid_argument("data");
	if (length < 1 || length > 128)
		throw std::invalid_argument("length");

	char* _data = new char[length];
	memcpy(_data, data, length);

	this->_data = _data;
	this->_length = length;
}

const char* KeyFingerprint::get() const
{
	return _data;
}

const int KeyFingerprint::length() const
{
	return _length;
}

bool KeyFingerprint::operator<(const KeyFingerprint& other) const
{
	return _length < other._length || memcmp(_data, other._data, _length) < 0;
}

bool KeyFingerprint::operator==(const KeyFingerprint& other) const
{
	return _length == other._length && memcmp(_data, other._data, _length) == 0;
}

KeyFingerprint& KeyFingerprint::operator=(const KeyFingerprint& other)
{
	delete[] _data;
	set(other._data, other._length);

	return *this;
}
