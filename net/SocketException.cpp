#include "network.hpp"

SocketException::SocketException(int number, const std::string& what)
	: Exception(what), _number(number)
{
}

int SocketException::number() const
{
	return _number;
}
