#include "network.hpp"

NetworkException::NetworkException(const char* what)
	: Exception(what)
{
}
