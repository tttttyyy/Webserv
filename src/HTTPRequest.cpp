#include "HTTPRequest.hpp"
#include "HTTPServer.hpp"

HTTPRequest::HTTPRequest(void)
{
    reqLineEnd = 0;
    bodyEnd = 0;
    _bodySize = 0;
    _isHeaderReady = false;
    _isBodyReady = false;
    _isRequestReady = false;
    _isInProgress = false;
    _isOpenConnection = false;
    _isCgi = false;
    _isChunked = false;
    _location = NULL;
}

HTTPRequest::~HTTPRequest() {}

std::string const &HTTPRequest::getMethod( void ) const {return (method);}

const std::string &HTTPRequest::getRequestBody() const {return (_body);}

std::string &HTTPRequest::getRequestBody() {return (_body);}

std::string const &HTTPRequest::getPath( void ) const {return (_relativePath);}

std::string const &HTTPRequest::getDisplayPath( void ) const {return (_path);};

std::string const &HTTPRequest::getVersion( void ) const {return (version);}

std::string const &HTTPRequest::getExtension() const {return (_extension);};

std::string const &HTTPRequest::getHttpRequest() const {return (httpRequest);}

std::map<std::string, std::string> &HTTPRequest::getUploadedFiles() {return (_uploadedFiles);}

bool HTTPRequest::isRequestReady() const {return (_isRequestReady);}

bool HTTPRequest::isBodyReady() const {return (_isBodyReady);}

bool HTTPRequest::isInProgress() const {return (_isInProgress);}

std::string HTTPRequest::rtrim(const std::string &str)
{
    size_t end = str.find_last_not_of(" \r\n\t\f\v");
    return (end == std::string::npos ? "" : str.substr(0, end + 1));
}

std::string HTTPRequest::ltrim(const std::string &str)
{
    size_t start = str.find_first_not_of(" \r\n\t\f\v");
    return (start == std::string::npos ? str : str.substr(start));
}

std::string HTTPRequest::trim(const std::string &str) {return (ltrim(rtrim(str)));}

std::string HTTPRequest::rtrim(const std::string &str, std::string const &set)
{
    size_t end = str.find_last_not_of(set);
    return (end == std::string::npos ? "" : str.substr(0, end + 1));
}

std::string HTTPRequest::ltrim(const std::string &str, std::string const &set)
{
    size_t start = str.find_first_not_of(set);
    return (start == std::string::npos ? str : str.substr(start));
}

std::string HTTPRequest::trim(const std::string &str, std::string const &set)
{
    return (ltrim(rtrim(str, set), set));
}


void HTTPRequest::charChange(std::string &str, char s, char d)
{
    for(size_t i = 0; i < str.size(); i++)
    {
        if (str[i] == s)
            str[i] = d;
    }
}

std::string HTTPRequest::findInMap(std::string key) const
{
    std::map<std::string, std::string>::const_iterator in = _httpHeaders.find(key);
    if (in != _httpHeaders.end()) {
        return (in->second);
    }
    std::string nill;
    return (nill);
}

void HTTPRequest::setBoundary() {
    std::map<std::string, std::string>::iterator it =  _httpHeaders.find("Content-Type");
    if(it == _httpHeaders.end())
    {
        throw ResponseError(500, "Internal Server Error");
    }
    if (it->second.find("multipart/form-data") != std::string::npos) {
        std::string contentType = it->second;
        contentType.erase(0, contentType.find(";") + 1);
        size_t posEqualsign = contentType.find("=");
        if (posEqualsign == std::string::npos) {
            return ;
        }
        _boundary = "--" + contentType.substr(posEqualsign + 1);
        _boundaryEnd = _boundary + "--";
    }
}

void HTTPRequest::showHeaders( void ) const
{
    std::ofstream osf("showHeaders.log");
    std::map<std::string, std::string>::const_iterator it;
    for(it = _httpHeaders.begin(); it != _httpHeaders.end(); it++)
    {
        osf << it->first << " = " << it->second << std::endl;
    }
}

std::string HTTPRequest::lastChar(std::string const &str, char s)
{
    std::string newString = str;
    if (!newString.empty())
    {
        size_t lst = newString.size() - 1;
        if (newString[lst] != s)
            newString += s;
    }
    return (newString);
}

void HTTPRequest::firstChar(std::string &str, char s)
{
    if (!str.empty())
        if (str[0] != s)
            str = s + str;
}

std::string HTTPRequest::middle_slash(std::string const &s1, char s, std::string const &s2)
{
    std::string newString;
    if (s1[s1.size()-1] == s && s2[0] != s)
        newString = s1 + s2;
    else if (s1[s1.size()-1] != s && s2[0] == s)
        newString = s1 + s2;
    else if (s1[s1.size()-1] != s && s2[0] != s)
        newString = s1 + s + s2;
    else if (s1[s1.size()-1] == s && s2[0] == s)
    {
        std::string ss2 = s2;
        ss2.erase(0, 1);
        newString = s1 + ss2;
    }
    return (newString);
}

// int HTTPRequest::in(std::string const &method)
// {
//     std::vector<std::string>::iterator it = std::find(_methods.begin(), _methods.end(), method);
//     if (it != _methods.end())
//         return (1);
//     return (0);
// }

std::string HTTPRequest::dir_content(std::string const &realPath)
{
    DIR* odir = opendir(realPath.c_str());
    std::string directoryContent;
    if (odir)
    {
        struct dirent* each;
        while ((each = readdir(odir)) != NULL)
        {
            std::string d_f_name = "<a href=\"" + std::string(each->d_name) + "\">" + std::string(each->d_name) + "</a><br>";
            directoryContent.insert(0, d_f_name);
        }
        return (directoryContent);
    }
    return (directoryContent);
}

void HTTPRequest::setExtension(const std::string &path) {
    (void)path;
    size_t pos = _relativePath.rfind(".");
    std::string tmpExtension = _relativePath.substr(pos + 1);
    if (tmpExtension.find("/") == std::string::npos) {
        _extension = HTTPRequest::trim(tmpExtension);
    }
}

void HTTPRequest::checkRedirect(const std::string &path, const std::string &redirectPath) {
    if (path == redirectPath) {
        throw ResponseError(508, "Loop Detected");
    }
    this->setRedirectPath(redirectPath);
    throw ResponseError(301, "Moved Permanently");
}

void appendSlash(std::string &str) {
    if (str.empty() == false && str[str.size() - 1] != '/') {
        str.append("/");
    }
}

void HTTPRequest::checkPath(const HTTPServer &srv)
{
    size_t use = 0;
    if ((use = _path.find_first_of("?")) != std::string::npos)
    {
        queryString = _path.substr(use+1);
        _path = _path.substr(0, use);
    }
    // std::cout << "_path = " << _path << std::endl;
    _location = srv.findMatching(_path);
    if (_location)
    {
        // std::cout << "_location = " <<  _location->getLocation() << std::endl;
        if (_location->getRedirection().empty() == false) {
            checkRedirect(_location->getLocation(), _location->getRedirection().begin()->second);
        }
        _path.erase(0, _location->getLocation().size());
        _relativePath = _location->getRoot();
        appendSlash(_relativePath);
        if (_relativePath.empty() == true) {
            _relativePath = "./";
        }
        _relativePath += _path;
        std::vector<std::string> indexes = _location->getIndexFiles();

        for (size_t i = 0; i < indexes.size(); i++) {
            std::string path = _relativePath;
            appendSlash(path);
            path += indexes[i];
            // std::cout << "path = " << path << std::endl;
            if (access(path.c_str(), R_OK) == 0) {
                _relativePath = path;
                break ;
            }
        }
        setExtension(_relativePath);
        if (_location->getCgi(_extension).first.empty() == false) {
            _isCgi = true;
        }
    }
    else {
        if (srv.getRedirection().empty() == false && _path == "/") {
            checkRedirect("/", srv.getRedirection().begin()->second);
        }
        _relativePath = srv.getRoot();
        if (_relativePath.empty() == true) {
            _relativePath = "./";
        }
        _relativePath += _path;
        if (_path == "/") {

            std::vector<std::string> indexes = srv.getIndexFiles();

            for (size_t i = 0; i < indexes.size(); i++) {
                std::string path = _relativePath + indexes[i];

                if (access(path.c_str(), R_OK) == 0) {
                    _relativePath = path;
                    break ;
                }
            }
        }
        setExtension(_relativePath);
        if (srv.getCgi(_extension).first.empty() == false) {
            _isCgi = true;
            this->setCgiPath(srv.getCgi(_extension).second);
        }
    }
    // std::cout << "_relativePath = " << _relativePath << std::endl;
}

std::vector<std::string> HTTPRequest::pathChunking(std::string const &rPath)
{
    std::string pathPrefix = rPath;
    std::vector<std::string> chunks;
    std::string pathChunk;
    for(size_t i = 0; i <= pathPrefix.size(); i++)
    {
        if ((pathPrefix[i] == '/' ||  i == pathPrefix.size()))
        {
            if (!pathChunk.empty())
            {
                chunks.push_back(pathChunk);
                pathChunk.clear();
            }
        }
        else
            pathChunk += pathPrefix[i];
    }
    return (chunks);
}

HTTPRequest::PathStatus HTTPRequest::path_status(bool autoindex, std::string const &checkPath)
{
    if (isExist(checkPath))
    {
        if (isDir(checkPath))
        {
            if (autoindex == true)
                return (ISDIR);
            else
                return (DIROFF);
        }
        else if (isFile(checkPath))
            return (ISFILE);
    }
    return (NOTFOUND);
}

size_t HTTPRequest::slashes(std::string const &pathtosplit)
{
    size_t count = 0;
    for(size_t i = 0; i < pathtosplit.size(); i++)
        if (pathtosplit[i] == '/')
            count++;
    return (count);
}

bool HTTPRequest::isDir(const std::string& filePath) {
    struct stat file;
    if (stat(filePath.c_str(), &file) != 0)
        return false;
    return S_ISDIR(file.st_mode);
}

bool HTTPRequest::isFile(const std::string& filePath) {
    struct stat file;
    if (stat(filePath.c_str(), &file) != 0)
        return false;
    return S_ISREG(file.st_mode);
}

bool HTTPRequest::isExist(std::string const &filePath)
{
    struct stat existing;
    return (stat(filePath.c_str(), &existing) == 0);
}

std::string const &HTTPRequest::getRedirectPath() const {return (_redirectPath);};

void HTTPRequest::setRedirectPath(const std::string &path) {_redirectPath = path;};

std::string const &HTTPRequest::getQueryString() const {return queryString;}

void HTTPRequest::setCgiPath(const std::string &cgiPath) {_cgiPath = cgiPath;};

std::string const &HTTPRequest::getCgiPath() const {return (_cgiPath);};

bool HTTPRequest::isCgi() const {return (_isCgi);};
