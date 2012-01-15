#include "exception.hpp"

FileNotFound::FileNotFound(const std::string& what)
	: Exception(what)
{
}
