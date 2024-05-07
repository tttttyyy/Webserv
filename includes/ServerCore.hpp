#ifndef SRV_CORE_HPP
#define SRV_CORE_HPP
#include "Libs.hpp"
#include "HTTPRequest.hpp"

class HTTPRequest;

struct ServerCore
{
    public:
        ServerCore();
        std::string const &getRoot( void ) const;
        std::vector<std::string> getIndexFiles( void ) const;
        std::vector<std::string> getMethods( void ) const;
        unsigned long getClientBodySize( void ) const;
        bool getAutoindex( void ) const;
    public:
        const char* findIndex(std::string const &filename) const;
        const char* findMethod(std::string const &method) const;
    public:
        void pushIndex(std::string const &fileNameExample);
        void dropIndexes(void);
        void pushMethods(std::string const &method);
        void pushErrPage(int, std::string const &errpage_filename);
        std::string getErrPage(int key) const;
        void setRoot(std::string const &root);
        void setAutoindex(std::string const &sw);
        void setSize(std::string const &bodySize);
        void setUploadDir(std::string const &upload_dir);
        std::string const &getUploadDir(void) const;
        void dropMethods();
    public:
        void setRedirection(int status, std::string redirectPath);
        void setR(bool status);
        std::map<int, std::string> const &getRedirection( void ) const;
        std::string getRedirection(int status) const;
        void setCgi(std::string cgiName, std::string cgiPath);
        std::pair<std::string, std::string> getCgi( std::string const &cgiType) const;
    protected:
        std::string root;                                       // [root]               www/server1/
        std::vector<std::string> index;                         // [index]              index.html barev.html index.htm ....
        std::vector<std::string> _methods;                       // [allow_methods]      GET | POST | DELETE
        std::map<int, std::string> error_page;                  // [error_page]         404 [root]/error_pages/404.html 
        bool _autoindex;                                        // [_autoindex]          on (true) | off (false)
        unsigned long int client_body_max_size;                     // [client_body_max_size]   200m -> 200.000.000byte -> 200mb
        bool _redirect;
        std::map<int, std::string> _redirections;
        std::map<std::string, std::string> _cgis;               // Example: cgi [php] [/usr/bin/php-cgi;]
        std::string _uploadDir;                                 // Example: upload_dir /Imgs/; 
};
#endif