#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP
#include "Libs.hpp"
#include <map>

class HTTPResponse
{
    public:
        HTTPResponse( void );
        ~HTTPResponse();
    public:
        std::string const getResponse( void ) const;
        void addHeader(const std::pair<std::string, std::string> &);
        void buildHeader();
        void setCgiPipeFd(int fd);
        std::string &getResponseBody();
        const std::string &getResponseHeader() const;
        const std::string &getResponseLine() const;
        void setBody(const std::string &body);
        bool isResponseReady() const;
        bool &isResponseReady();
        bool isStarted() const;
        void setStartStatus(bool);
        void setResponseLine(std::string const &line);
        bool isErrorResponse() const;
        void setIsErrorResponse(bool isErrorResponse);
    protected:
        std::string _header;
        std::string _responseLine;
        std::string _responseBody;
        std::map<std::string, std::string> _responseHeader;
        bool _isResponseReady;
        bool _isStarted;
        bool _isErrorResponse;
    private:
        std::string reserve;
};

#endif