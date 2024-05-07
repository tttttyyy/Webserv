#pragma once
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <Client.hpp>

#ifdef __linux__
# define PHP_CGI_PATH "./CGI/php-cgi-linux"
# define PYTHON_CGI_PATH "./CGI/python-cgi-linux"
#else
# define PHP_CGI_PATH "./CGI/php-cgi-mac"
# define PYTHON_CGI_PATH "./CGI/python-cgi-mac"
#endif

#define CGI_TIMEOUT 120 // sec


class Cgi
{
    public:
        static int execute(Client &client);
        static char **initEnv(Client const &client);
    private:
        Cgi();
        Cgi(const Cgi &rhs);
        Cgi &operator=(const Cgi &rhs);
        ~Cgi();
    private:
        static std::map<std::string, std::string> _env;
};