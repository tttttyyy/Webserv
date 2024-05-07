#include "Tcp.hpp"

Tcp::Tcp( void ) : _fd(-1), backlog(-1)
{
	memset(&this->rules, 0, sizeof(rules));
	memset(&this->Socket, 0, sizeof(Socket));
	memset(&this->SocketAddress, 0, sizeof(SocketAddress));
	memset(&this->clntAddr, 0, sizeof(clntAddr));
    memset(&this->addrList, 0, sizeof(addrList));
}

Tcp::~Tcp() {freeaddrinfo(addrList);}

// const char* Tcp::pton(uint32_t ipv) const //woncwor hesa bayc chi ogtagorcvum
// {
//     char *ip = new char[16];
//     std::stringstream ss;
//     ss << ((int)(ipv >> 24) & 0XFF) << "." << ((int)(ipv >> 16) & 0XFF) << "." << ((int)(ipv >> 8) & 0XFF) << "." << ((int)(ipv) & 0XFF);
//     int i = -1;
//     while (++i < (int)ss.str().size())
//         ip[i] = ss.str().c_str()[i];
//     ip[i] = '\0';
//     return (ip);
// }

void Tcp::setup(const char* ip, const char* port)
{
    int addrinfo = 0;
	rules.ai_family = PF_UNSPEC;
	rules.ai_flags = AI_PASSIVE;
	rules.ai_socktype = SOCK_STREAM;
	rules.ai_protocol = 0;
	rules.ai_canonname = NULL;
	if ((addrinfo = getaddrinfo(ip, port, &rules, &addrList)) < 0)
        throw HTTPCoreException(gai_strerror(addrinfo));
    SocketAddress = addrList->ai_addr;
}

void Tcp::createSocket( void )
{
    _fd = socket(addrList->ai_family, addrList->ai_socktype, addrList->ai_protocol);
    if (_fd < 0)
        throw HTTPCoreException(strerror(errno));
}

void Tcp::bindSocket( void )
{
    int l = 1;
    setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &l, sizeof(l));
    if (fcntl(_fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC) < 0)
        throw HTTPCoreException(strerror(errno));
    if (bind(_fd, SocketAddress, addrList->ai_addrlen) < 0)
        throw HTTPCoreException(strerror(errno));
}

void Tcp::listenSocket( void )
{
    if (listen(_fd, 32) < 0)
        throw HTTPCoreException(strerror(errno));
}

sock_t Tcp::accept( void )
{
    socklen_t clntSize = sizeof(clntAddr);
    sock_t client = ::accept(_fd, &clntAddr, &clntSize);
    // struct sockaddr_in* cl = (struct sockaddr_in*)&clntAddr;
    // std::cout << inet_ntoa(cl->sin_addr) << std::endl;
    if (fcntl(client, F_SETFL, O_NONBLOCK, FD_CLOEXEC) < 0)
        throw HTTPCoreException((std::string("fcntl: ") + strerror(errno)).c_str());
    return (client);
}

struct sockaddr_in* Tcp::getClientAddress( void )
{
    return ((struct sockaddr_in*)&this->clntAddr);
}