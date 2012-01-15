#include "network.hpp"

NetworkException::NetworkException(const std::string& what)
	: Exception(what)
{
}
