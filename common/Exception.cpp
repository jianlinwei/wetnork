#include "exception.hpp"

Exception::Exception(const char* what)
	: _what(what)
{
}

const char* Exception::what() const throw()
{
	return _what;
}
