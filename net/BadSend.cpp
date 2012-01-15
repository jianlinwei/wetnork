#include "network.hpp"

BadSend::BadSend(const std::string& what)
	: NetworkException(what)
{
}
