#ifndef CLIENT_HPP
#define CLIENT_HPP
#include "Libs.hpp"
#include "HTTPRequest.hpp"
#include "ServerManager.hpp"
#include <ctime>
#include "utils.hpp"
#include <signal.h>
#include "EvManager.hpp"
#include "InnerFd.hpp"

#define LAST_SENN_RIMEOUT 15 // sec

struct InnerFd;

class HTTPServer;
class Client : public HTTPRequest, public HTTPResponse
{
    public:
        Client(sock_t clfd, sock_t srfd, HTTPServer &srv);
        ~Client();
    public:
        void setSocketAddress(struct sockaddr_in *addr);
        const struct sockaddr_in* getSocketAddress( void ) const;
        char* inet_ntoa(struct in_addr) const;
        std::string getUser(std::string const &pwd) const;
        sock_t getFd( void ) const;
        sock_t getServerFd( void ) const;
        std::string getServerPort( void ) const;
        const std::string &getTmpToChild() const;
        int receiveRequest();
        void parseHeader();
        void parseBody();
        int sendResponse();
        const HTTPServer &getSrv( void ) const;
        HTTPServer &getSrv( void );
        HTTPServer &getDefaultSrv( void );
        void setCgiStartTime();
        bool checkCgi();
        void setCgiPipeFd(int fd);
        void setCgiPID(int fd);
        InnerFd *getInnerFd(int fd);
        void addInnerFd(InnerFd *);
        void removeInnerFd(int fd);
        const ServerCore &getCurrentLoc() const;
    private:
        bool readChunkedRequest();
        void multipart(void);
        std::map<int, InnerFd *> _innerFds;                   // [Clients inner fds]
        std::vector<std::string> _tmpFiles;
        sock_t _fd;
        sock_t serverFd;
        HTTPServer &_defaultSrv;
        HTTPServer *_subSrv;
        std::time_t	 _lastSeen;
        std::time_t	 _cgiStartTime;
        int _cgiPipeFd;
        int _cgiPID;
        struct sockaddr_in clientInfo;
    private:
        Client &operator=(const Client &);
        Client(const Client &);
    private:
        bool _isChunkStarted;
        size_t _chunkSize;
};

#endif