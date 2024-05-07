#ifndef REQ_HPP
#define REQ_HPP
#include "Libs.hpp"
#include "HTTPResponse.hpp"
#define MAX_SIZE_REQUEST 1000
#include <map>

class Location;
class HTTPServer;
class HTTPRequest
{
	public:
		HTTPRequest( void );
		~HTTPRequest();
    public:
        std::string findInMap(std::string key) const;
        void showHeaders( void ) const;
        std::string const &getMethod( void ) const;
        std::string const &getPath( void ) const;
        std::string const &getDisplayPath( void ) const;
        std::string const &getVersion( void ) const;
        std::string const &getHttpRequest() const;
        std::string const &getExtension() const;
        std::string const &getRedirectPath() const;
        std::string const &getQueryString() const;
        void setRedirectPath(const std::string &path);
        void setCgiPath(const std::string &cgiPath);
        std::string const &getCgiPath() const;
        const std::string &getRequestBody() const;
        std::string &getRequestBody();
        bool isRequestReady() const;
        bool isInProgress() const;
        bool isBodyReady() const;
        bool isCgi() const;
    public:
        static bool isDir(std::string const &filePath);
        static bool isFile(std::string const &filePath);
        static bool isExist(std::string const &filePath);
        static void charChange(std::string &str, char s, char d);
        static std::string lastChar(std::string const &str, char s);
        static void firstChar(std::string &str, char s);
        static std::string middle_slash(std::string const &s1, char s, std::string const &s2);
        static size_t slashes(std::string const &pathtosplit);
        static std::string ltrim(const std::string &str);
        static std::string rtrim(const std::string &str);
        static std::string trim(const std::string &str);
        static std::string ltrim(const std::string &str, std::string const &set);
        static std::string rtrim(const std::string &str, std::string const &set);
        static std::string trim(const std::string &str, std::string const &set);
    protected:
        std::vector<std::string> pathChunking(std::string const &rPath);
        void checkPath(HTTPServer const &srv);
        void setBoundary(void);
        // int in(std::string const &method);
    protected:
        std::vector<std::string> pathChunks;
        enum PathStatus{ISDIR, DIROFF, DIRON, ISFILE, NOTFOUND, FORBIDDEN, UNDEFINED};
        PathStatus path_status(bool autoindex, std::string const &checkPath);
        PathStatus pathinfo;
    protected:
        size_t reqLineEnd;
        size_t bodyEnd;
        std::string httpRequest;
        std::string request;
        std::string method;
        std::string _path;
        std::string realPath;
        std::string actualPath;
        std::string _relativePath;
        std::string reqStatus;
        std::string _fileName;
        std::string _extension;
        std::string queryString;
        std::string version;
        std::string headers;
        std::string _body;
        std::string _requestBuf;
    protected:
        std::string _redirectPath;
    protected:
        std::map<std::string, std::string> _httpHeaders;
    protected:
        std::string _boundary;
        std::string _boundaryEnd;
        std::string _contentType;
        size_t _bodySize;
        size_t _acceptedBodySize;
    protected:
        const Location* _location;
    protected:
        std::vector<std::string> _methods;
    protected:
        std::string _cgiPath;
        
    protected:
        std::string dir_content(std::string const &realPath);
        void setExtension(const std::string &path);
        void checkRedirect(const std::string &path, const std::string &redirectPath);
    protected:
        bool _isInProgress;
        bool _isHeaderReady;
        bool _isBodyReady;
        bool _isRequestReady;
        bool _isOpenConnection;
        bool _isCgi;
        bool _isChunked;
        std::map<std::string, std::string> _uploadedFiles;
    public:
        std::map<std::string, std::string> &getUploadedFiles();

};

#endif