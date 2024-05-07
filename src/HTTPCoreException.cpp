#include "HTTPCoreException.hpp"

HTTPCoreException::HTTPCoreException(const char* msg) : err(msg) {}

HTTPCoreException::~HTTPCoreException() throw() {}

const char* HTTPCoreException::what( void ) const throw() {return (this->err.c_str());}