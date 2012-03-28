#include <exception.hpp>

InvalidOperation::InvalidOperation(const std::string& what)
	: Exception(what)
{
}
