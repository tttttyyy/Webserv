#include "ResponseError.hpp" 
 
ResponseError::ResponseError(int statusCode, const std::string &errMessage)
    : _statusCode(statusCode), _errMessage(errMessage), _client(NULL) {};

ResponseError::ResponseError(int statusCode, const std::string &errMessage, Client &client)
    : _statusCode(statusCode), _errMessage(errMessage), _client(&client) {};

ResponseError::ResponseError(const ResponseError& rhs)
    : _statusCode(rhs._statusCode), _errMessage(rhs._errMessage), _client(rhs._client)  {};

ResponseError::~ResponseError() throw() {}

const char * ResponseError::what() const throw() {return _errMessage.c_str();}

int ResponseError::getStatusCode() const {return (_statusCode);}

Client *ResponseError::getClient() {return (_client);}