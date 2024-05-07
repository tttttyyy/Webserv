#ifndef SERVER_MANAGER
#define SERVER_MANAGER
#include "Libs.hpp"
#include "Parser.hpp"
#include "Location.hpp"
#include "HTTPServer.hpp"
#include "Client.hpp"
#include "ResponseError.hpp"
#define SUCCSSES_STATUS "OK"

class Client;
class HTTPServer;
class Error;
class ResponseError;

class ServerManager : public std::vector<HTTPServer *>
{
    public:
        ServerManager(const char *configfile);
        ~ServerManager();
        void clearInstances( void );
    public:
        void printFds( void );
    public:
        HTTPServer  *getServerBySocket(sock_t fd); 
        HTTPServer  *getServerByClientSocket(sock_t fd);
        sock_t getmax( void ) const;
        int used(HTTPServer &srv) const;
    public:
        std::vector<HTTPServer> getVirtualServers( void );
    public:
        void start();
        void generateResponse(Client &client);
        void generateErrorResponse(const ResponseError& e, Client &client);
    private:
        bool closeConnetcion(Client &client);
        bool newClient(int fd);
};

#endif
