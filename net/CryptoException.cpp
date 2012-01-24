#include "crypto.hpp"

CryptoException::CryptoException(int code, int alert, const std::string& what)
	: Exception(what), _code(code), _alert(alert)
{
}

int CryptoException::code() const
{
	return _code;
}

int CryptoException::alert() const
{
	return _alert;
}
