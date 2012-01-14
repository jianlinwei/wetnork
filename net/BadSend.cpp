#include "network.hpp"

BadSend::BadSend(const char* what)
	: NetworkException(what)
{
}
