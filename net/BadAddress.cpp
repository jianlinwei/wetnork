#include "network.hpp"

BadAddress::BadAddress(const std::string& what)
	: NetworkException(what)
{
}
