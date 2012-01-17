#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>
#include <string>

class Exception : public std::exception {
	private:
		std::string _what;

	public:
		Exception(const std::string& what);

		~Exception() throw();

		const char* what() const throw();
};

class FileNotFound : public Exception {
	public:
		FileNotFound(const std::string& what);
};


class InvalidOperation : public Exception {
	public:
		InvalidOperation(const std::string& what);
};

#endif
