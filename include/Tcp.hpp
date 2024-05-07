#pragma once
#include "Libs.hpp"
class Tcp
{
	public:
		Tcp( void );
		~Tcp();
	public:
		sock_t accept( void );
		struct sockaddr_in* getClientAddress( void );
	protected:
        // const char* pton(uint32_t ipv) const; //woncwor hesa bayc chi ogtagorcvum
		void setup(const char* ip, const char* port);
		void createSocket( void );
		void bindSocket( void );
		void listenSocket( void );
	protected:
		int _fd;
		int backlog;
	protected:
		struct addrinfo rules;
		struct addrinfo* addrList;
		struct sockaddr_in *Socket;
		struct sockaddr *SocketAddress;
		struct sockaddr clntAddr;
};