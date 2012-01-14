#include "network.hpp"

BadAddress::BadAddress(const char* what)
	: NetworkException(what)
{
}
