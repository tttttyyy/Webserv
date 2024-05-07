#ifndef HTTP_CORE_EXCEPTIONS_HPP
#define HTTP_CORE_EXCEPTIONS_HPP
#include "Libs.hpp"

class HTTPCoreException : public std::exception
{
	public:
		HTTPCoreException(const char *msg);
		virtual ~HTTPCoreException() throw();
		virtual const char* what( void ) const throw();
	private:
		std::string err;
};

#endif