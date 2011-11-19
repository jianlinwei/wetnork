#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>

class Exception : public std::exception {
	private:
		const char* _what;

	public:
		Exception(const char* what)
			: _what(what)
		{}

		const char* what() const throw() { return _what; }
};

#endif
