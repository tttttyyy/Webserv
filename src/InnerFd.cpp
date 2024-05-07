#include "InnerFd.hpp"

InnerFd::InnerFd(int fd, Client  &client, std::string &str, int flag)
    : _client(client), _str(str)
{
    if (fd < 3) {
        throw std::logic_error("fd < 3");
    }
    _fd = fd;
    _flag = flag;
}

InnerFd::InnerFd(const InnerFd &obj) : _client(obj._client), _str(obj._str)
{
    _fd = obj._fd;
    _flag = obj._flag;
};

InnerFd::~InnerFd(){};