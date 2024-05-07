#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP
#include "Libs.hpp"
#include "Tcp.hpp"
#include "IListener.hpp"
#include "ServerCore.hpp"
#include "ServerManager.hpp"
#include "ResponseError.hpp"
#include <errno.h>
#include "InnerFd.hpp"

struct InnerFd;

class HTTPRequest;
class ServerManager;
class Location;
class Client;
class HTTPServer : public Tcp, public IListener, public ServerCore
{
    public:
        HTTPServer( void );
        virtual ~HTTPServer();
    public:
		void up();
        sock_t getfd( void ) const;
    public:
        Client *getClient(sock_t fd);
        InnerFd *getInnerFd(int fd);
        void readFromFd(int fd, std::string &str);
        void writeInFd(int fd, std::string &str);
    public:
        void push(std::string const &prefix, Location const &locationDirective);
        void push(sock_t clFd, Client *clt);
        void removeClient(sock_t fd);
        void push(HTTPServer *srv);
        void push__serverName(std::string const &srvName);
    public:
        const Location *find(std::string const &prefix) const;
        const Location* findMatching(std::string const &realPath) const;
        bool exist(sock_t fd);
        std::vector<std::string> const &get_serverNames( void ) const;
        std::map<std::string, Location> const &getLocations( void ) const;
        HTTPServer *getSubServerByName(std::string const &_serverName);
    private:
        std::vector<HTTPServer *> _srvs;
        std::vector<std::string> _serverName;
        std::map<sock_t, Client *> _clnt;                   // [Clients]
        std::map<std::string, Location> _locations;      // <prefix, LocationDirective>  location / {Location}
    public: //ip port interface
		virtual void setPort(std::string const &port);
		virtual void setIp(std::string const &ipv);
		virtual const char* getIp( void ) const;
		virtual const char* getPort( void ) const;
        // virtual uint32_t getNIp( void ) const;
		// virtual uint16_t getNPort( void ) const;
        std::string	directory_listing(const std::string &path, std::string displayPath);
    public:
        bool operator==(HTTPServer const &) const;
        bool operator==(sock_t) const;
    public:
        void get(Client &client);
        void post(Client &client);
        void del(Client &client);
        // void head(Client &client);
        void processing(Client &client);

        std::string executeCgi(Client &client);
    private:
        std::map<std::string, void (HTTPServer::*)(Client&)> methodsMap;
};

#endif