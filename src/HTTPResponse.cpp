#include "HTTPResponse.hpp"

HTTPResponse::HTTPResponse( void )
{
    _responseHeader["Server"] = "webserv";
    _responseHeader["Accept-Ranges"] = "bytes";
    _isResponseReady = false;
    _isStarted = false;
    _isErrorResponse = false;
}

HTTPResponse::~HTTPResponse() {}

bool HTTPResponse::isErrorResponse() const {return (_isErrorResponse);} 

void HTTPResponse::setIsErrorResponse(bool isErrorResponse) {_isErrorResponse = isErrorResponse;}

std::string const HTTPResponse::getResponse( void ) const {return (_responseLine + _header + _responseBody);}

void HTTPResponse::setResponseLine(std::string const &line) {_responseLine = line;}

void HTTPResponse::addHeader(const std::pair<std::string, std::string> &pair) {_responseHeader[pair.first] = pair.second;}

const std::string &HTTPResponse::getResponseHeader() const {return (_header);}

void HTTPResponse::buildHeader()
{
    for (std::map<std::string, std::string>::iterator it = _responseHeader.begin();
        it != _responseHeader.end(); ++it)
        _header += it->first + ": " + it->second + "\r\n";
    _header += "\r\n";
}

std::string &HTTPResponse::getResponseBody() {return (_responseBody);}

const std::string &HTTPResponse::getResponseLine() const {return (_responseLine);}

void HTTPResponse::setBody(const std::string &body)
{
    _responseBody = body;
    _isResponseReady = true;
}

bool HTTPResponse::isResponseReady() const {return (_isResponseReady);}

bool &HTTPResponse::isResponseReady() {return (_isResponseReady);}

bool HTTPResponse::isStarted() const {return (_isStarted);}

void HTTPResponse::setStartStatus(bool is) {_isStarted = is;}