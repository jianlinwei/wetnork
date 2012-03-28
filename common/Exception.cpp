#include <exception.hpp>

using namespace std;

Exception::Exception(const string& what)
	: _what(what)
{
}

Exception::~Exception() throw()
{
}

const char* Exception::what() const throw()
{
	return _what.c_str();
}
