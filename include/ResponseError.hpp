#pragma once
#include <iostream>
#include "Client.hpp"

class Client;
class ResponseError : public std::exception
{
    public:
        ResponseError(int statusCode, const std::string &errMessage);
        ResponseError(int statusCode, const std::string &errMessage, Client &client);
        ResponseError(const ResponseError& rhs);
        ~ResponseError() throw();

        const char * what() const throw();

        int getStatusCode() const;

        Client *getClient();

    private:
        ResponseError();
        ResponseError& operator=(const ResponseError& rhs);
    private:
        int _statusCode;
        std::string _errMessage;
        Client *_client;
};