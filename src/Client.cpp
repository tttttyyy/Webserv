#include "Client.hpp"
#include "ResponseError.hpp"
#include "InnerFd.hpp"
#include <sys/types.h>
#include <sys/wait.h>
#include "Cgi.hpp"

Client::Client(sock_t clfd, sock_t srfd, HTTPServer &srv) : _defaultSrv(srv)
{
    this->_fd = clfd;
    this->serverFd = srfd;
    _subSrv = NULL;
    _cgiPipeFd = -1;
    _cgiPID = -1;
    _acceptedBodySize = 0;
    _lastSeen = time(NULL);
    _isChunkStarted = false;
    _chunkSize = 0;
}

Client::~Client()
{
    std::map<int, InnerFd *>::iterator it = _innerFds.begin();
    while (it != _innerFds.end()) {
        delete it->second;
        EvManager::delEvent(it->first, EvManager::read);
        EvManager::delEvent(it->first, EvManager::write);
        close(it->first);
        ++it;
    }
    for (size_t i = 0; i < _tmpFiles.size(); i++) {
        std::remove(_tmpFiles[i].c_str());
    }
}

sock_t Client::getFd( void ) const
{
    return (this->_fd);
}

sock_t Client::getServerFd( void ) const
{
    return (this->serverFd);
}

std::string Client::getServerPort( void ) const {
    return (_defaultSrv.getPort());
};

const std::string &Client::getTmpToChild() const {
   return(_tmpFiles[0]); 
}

// std::ofstream ofs("_requestBuf.log");

void Client::multipart(void)
{
    size_t posHeaderEnd = _body.find("\r\n\r\n");
    while (posHeaderEnd != std::string::npos
            || (_isChunkStarted == true && _body.empty() == false)) {
        if (posHeaderEnd != std::string::npos && _isChunkStarted == false) {
            size_t filenameStart = _body.find("filename");

            if (filenameStart != std::string::npos) {
                filenameStart += strlen("filename") + 2;
                _fileName = _body.substr(filenameStart, _body.find("\"", filenameStart) - filenameStart);
                _body.erase(0, posHeaderEnd + strlen("\r\n\r\n"));
                int fd = open((this->getCurrentLoc().getUploadDir() + _fileName).c_str(),  O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU);
                if (fd == -1) {
                    throw ResponseError(500, "Internal Server Error");
                }
                EvManager::addEvent(fd, EvManager::write, EvManager::inner);
                this->addInnerFd(new InnerFd(fd, *this,  _uploadedFiles[_fileName], EvManager::write));
                _isChunkStarted = true;
            } else {
                throw ResponseError(428, "Precondition Required posEqualsign");
            }
        }
        if (_isChunkStarted == true) {
            size_t secondBoundaryPos = std::string::npos;

            size_t pos = _body.find("\r\n");
            if (pos != std::string::npos) {
                if (((_body.size() - pos) < _boundary.size() + strlen("\r\n"))) {
                    return ;
                }
                secondBoundaryPos = _body.find(_boundary);
            }
            size_t cutLen = 0;

            if (secondBoundaryPos != std::string::npos) {
                cutLen = secondBoundaryPos;
                _isChunkStarted = false;
            } else {
                cutLen = _body.size();
            }
            _uploadedFiles[_fileName].append(_body.substr(0, cutLen));
            _body.erase(0, cutLen);
        }
        posHeaderEnd = _body.find("\r\n\r\n");
    }
}

bool Client::readChunkedRequest() {
    char *ptr;
    // std::cout << "readChunkedRequest\n";
    size_t pos = _requestBuf.find("\r\n");
    while ((pos != std::string::npos)
        || (_isChunkStarted == true && _requestBuf.empty() == false)) {
        if (pos != std::string::npos && pos == 0) {
            _requestBuf.erase(0, strlen("\r\n"));
            pos = _requestBuf.find("\r\n");
            continue ;
        }
        if (_isChunkStarted == false) {
            _chunkSize =  std::strtoul(_requestBuf.c_str(), &ptr, 16); // TODO check with client body max size
            // ofs << "_chunkSize = " << _chunkSize << std::endl;
            if ((_body.size() + _chunkSize) > this->getCurrentLoc().getClientBodySize()) {
                throw ResponseError(413, "Content Too Large");
            }
            if (_chunkSize == 0) {
                return true;
            }
            size_t posEndl = _requestBuf.find("\r\n");
            _requestBuf.erase(0, posEndl + strlen("\r\n"));
            _isChunkStarted = true;
        } 
        if (_isChunkStarted == true) {
            size_t existChunkSize = _chunkSize < _requestBuf.size() ? _chunkSize : _requestBuf.size();
            _body.append(_requestBuf.c_str(), existChunkSize);
            _requestBuf.erase(0, existChunkSize);
            _chunkSize -= existChunkSize;
            if (_chunkSize == 0) {
                _isChunkStarted = false;
            }
        }
        pos = _requestBuf.find("\r\n");
    }
    return false;
}

int Client::receiveRequest() {
    char buf[READ_BUFFER];
    int rdSize = recv(_fd, buf, sizeof(buf), 0);
    // std::cout << "rdSize = " << rdSize << std::endl;
    if (rdSize == 0 || rdSize == -1) {
        return (-1);
    }
    _requestBuf.append(buf, rdSize);
    if (_isHeaderReady == false) {
        size_t headerEndPos = _requestBuf.find("\r\n\r\n");
        if (headerEndPos == std::string::npos) {
            return 0;
        }
        _isHeaderReady = true;
        httpRequest  = _requestBuf.substr(0, headerEndPos);
        _requestBuf.erase(0, headerEndPos + strlen("\r\n\r\n"));
        this->parseHeader();
        this->showHeaders();
        std::map<std::string, std::string>::const_iterator it = _httpHeaders.find("Transfer-Encoding");
        if (it != _httpHeaders.end() && (it->second.find("Chunked") != std::string::npos ||  it->second.find("chunked") != std::string::npos)) {
            if (it->second.find("Chunked") != std::string::npos ||  it->second.find("chunked") != std::string::npos) {
                _isChunked = true;
                // std::cout << "_isChunked = true" << std::endl;
            } else {
                throw ResponseError(400, "Bad Request");
            }
        }
        it = _httpHeaders.find("Content-Length");
        if (it != _httpHeaders.end()) {
            char *ptr;
            _bodySize = std::strtoul(it->second.c_str(), &ptr, 10);
            // std::cout << "_bodySize = " << _bodySize << std::endl;
            if (_bodySize > this->getCurrentLoc().getClientBodySize() || it->second.size() > 19) {
                throw ResponseError(413, "Content Too Large");
            }
            this->setBoundary();
        } else if (_isChunked == false && this->getMethod() == "POST") {
            throw ResponseError(411 , "Length Required");
        }
        if (_isCgi == true && getMethod() == "POST") {
            std::string tmpFile = "./.tmp/to_child" + my_to_string(_fd);
            if (access("./.tmp", F_OK) != 0) {
                if (mkdir("./.tmp", S_IRWXU) == -1) {
                    throw std::runtime_error(std::string("mkdir: ") + strerror(errno));
                };
            }
            _tmpFiles.push_back(tmpFile);
            int fd = open(tmpFile.c_str(),  O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU);
            // std::cout << tmpFile << " = " << fd << std::endl;
            EvManager::addEvent(fd, EvManager::write, EvManager::inner);
            this->addInnerFd(new InnerFd(fd, *this, _body, EvManager::write));
        }
    }
    if (_isHeaderReady == true ) {
        _isInProgress = true;
        if (_isChunked ) {
            if (readChunkedRequest() == true) {
            _isBodyReady = true;
            _isRequestReady = true;
            }
            return (0);
        } else  if (_bodySize == 0 || this->getMethod() == "GET") {
            _isBodyReady = true;
            _isRequestReady = true;
            _requestBuf.clear();
            return (0);
        } else {
            _body.append(_requestBuf.c_str(), _requestBuf.size());
            _acceptedBodySize += _requestBuf.size();
            _requestBuf.clear();
            if (_bodySize <= _acceptedBodySize) {
                _body.erase(_body.size() - (_acceptedBodySize - _bodySize));
                // std::cout << "_isRequestReady = true;\n";
                _isBodyReady = true;
                _isRequestReady = true;
            }
            if (_isCgi == false && this->getMethod() == "POST" && _contentType.find("multipart/form-data") != std::string::npos) {
                this->isStarted();
                this->multipart();
                if (_isBodyReady == true) {
                    this->getResponseBody() = "ok";
                    this->addHeader(std::pair<std::string, std::string>("Content-Length", my_to_string(this->getResponseBody().size())));
                    this->buildHeader();
                    this->isResponseReady() = true;
                }
            } else if (_isCgi == false) {
                throw ResponseError(501, "Not Implemented");
            }
        }
    }
    return 0;
}

// std::ofstream ofsParseHeader("ofsParseHeader.log");

void Client::parseHeader()
{
    // ofsParseHeader << "\n\n\n\n\n$httpRequest.size() = " << httpRequest.size() << "\n";
    // ofsParseHeader << "$" << httpRequest << "$\n\n\n\n\n";
    size_t space = 0;
    size_t pos = httpRequest.find("\r\n");
    // ofsParseHeader << "pos = " << pos << std::endl;
    request = httpRequest.substr(0, pos);
    httpRequest.erase(0, pos + 2);

    for (size_t i = 0; i < request.size(); i++)
        if (std::isspace(request[i]))
            space++;
    // ofsParseHeader << "space = " << space << std::endl;
    if (space >= 2)
    {
        method = trim(request.substr(0, request.find_first_of(" ")));
        request.erase(0, request.find_first_of(" ") + 1);
        _path = trim(request.substr(0, request.find_first_of(" ")));
        if (_path.size() > 2048) {
            throw ResponseError(414, "URI Too Long");
        }
        request.erase(0, request.find_first_of(" ") + 1);
        version = trim(request.substr(0, request.find("\r\n")));
    }
    std::stringstream iss(httpRequest);
    std::string get_next_line;

    while (std::getline(iss, get_next_line) && get_next_line != "\r\n")
    {
        size_t colon;
        if ((colon = get_next_line.find_first_of(":")) != std::string::npos && std::isspace(get_next_line[colon+1]))
        {
            std::string key = trim(get_next_line.substr(0, colon));
            std::string value = trim(get_next_line.substr(colon+2, get_next_line.find("\r\n")));
            _httpHeaders.insert(std::make_pair(key, value));
        }
    }
    httpRequest.clear();
    std::map<std::string, std::string>::iterator it = _httpHeaders.find("Host");
    if (it != _httpHeaders.end()) {
        _subSrv = _defaultSrv.getSubServerByName(it->second);
    }
    HTTPRequest::checkPath(this->getSrv());
    _contentType = _httpHeaders["Content-Type"];
}

int Client::sendResponse() {
    if (_responseLine.empty() == false) {
        size_t sendSize = WRITE_BUFFER < _responseLine.size() ? WRITE_BUFFER : _responseLine.size();
        int res = send(_fd, _responseLine.c_str(), sendSize, 0);
        if (res == -1 || res == 0) {
            return (-1);
        }
        _responseLine.erase(0, res);
    } else if (_header.empty() == false) {
        size_t sendSize = WRITE_BUFFER < _header.size() ? WRITE_BUFFER : _header.size();
        int res = send(_fd, _header.c_str(), sendSize, 0);
        if (res == -1 || res == 0) {
            return (-1);
        }
        _header.erase(0, res);
    } else if (_responseBody.empty() == false) {
        size_t sendSize = WRITE_BUFFER < _responseBody.size() ? WRITE_BUFFER : _responseBody.size();
        int res = send(_fd, _responseBody.c_str(), sendSize, 0);
        if (res == -1 || res == 0) {
            return (-1);
        }
        _responseBody.erase(0, res);
    }
    return (_responseBody.empty() && _header.empty() && _responseLine.empty());
}

const HTTPServer &Client::getSrv( void ) const {
    if (_subSrv) {
        return (*_subSrv);
    }
    return (_defaultSrv);
};

HTTPServer &Client::getDefaultSrv( void ) {
    return (_defaultSrv);
};

HTTPServer &Client::getSrv( void ) {
    if (_subSrv) {
        return (*_subSrv);
    }
    return (_defaultSrv);
};


void Client::setCgiStartTime() {
    _cgiStartTime = time(NULL);
};

bool Client::checkCgi() {
    if (_cgiPID != -1) {
        // std::cout << ":checkCgi\n";
        int status;
        int waitRet;
        waitRet = waitpid(_cgiPID, &status, WNOHANG);
        if (waitRet == -1) {
            throw ResponseError(500, "Internal Server Error");
        }
        if (waitRet == 0 && time(NULL) - _cgiStartTime > CGI_TIMEOUT) {
            if (kill(_cgiPID, SIGKILL) == -1) {
                throw std::runtime_error(std::string("kill: ") + strerror(errno));
            };
            waitpid(_cgiPID, &status, 0);
            _cgiPID = -1;
            if (WTERMSIG(status) == SIGKILL) {
                throw ResponseError(508, "Loop Detected");
            }
            throw ResponseError(500, "Internal Server Error");
        }
        if (waitRet != 0 && WIFEXITED(status)) {
            _cgiPID = -1;
            // std::ofstream osf("cgi_output.log");
            // char buf[2000];
            // buf[read(_cgiPipeFd, buf, 1999)] = '\0';
            // osf << buf;
            if (WEXITSTATUS(status) != 0) {
                // osf << this->getRequestBody();
                // std::cout << "WEXITSTATUS(status) = " << WEXITSTATUS(status) << std::endl;
                throw ResponseError(500, "Internal Server Error");
            }
            // std::cout << "WEXITSTATUS(status) = " << WEXITSTATUS(status) << std::endl;
            EvManager::addEvent(_cgiPipeFd, EvManager::read, EvManager::inner);
            this->addInnerFd(new InnerFd(_cgiPipeFd, *this, this->getResponseBody(), EvManager::read));
        }
    }
    return (true);
};


void Client::setCgiPipeFd(int fd) {
    _cgiPipeFd = fd;
};

void Client::setCgiPID(int pid) {
    _cgiPID = pid;
};

const ServerCore &Client::getCurrentLoc() const {
    if (_location) {
        // std::cout << "_location = " << _location->getLocation() << std::endl;;
        return (*_location);
    }
    if (_subSrv) {
        // std::cout << "_subSrv\n";
        return (*_subSrv);
    }
        // std::cout << "_defaultSrv\n";
    return (_defaultSrv);
};


InnerFd *Client:: getInnerFd(int fd) {
    std::map<int, InnerFd *  >::iterator it = _innerFds.find(fd);
    if (it != _innerFds.end()) {
        return(it->second);
    }
    return (NULL);
};

void Client::addInnerFd(InnerFd *obj) {
    if (obj) {
        _innerFds.insert(std::pair<int, InnerFd *>(obj->_fd, obj));
    }
};

void Client::removeInnerFd(int fd) {
    std::map<int, InnerFd *>::iterator it = _innerFds.find(fd);
    if (it != _innerFds.end()) {
        delete it->second;
        _innerFds.erase(fd);
        EvManager::delEvent(fd, EvManager::read);
        EvManager::delEvent(fd, EvManager::write);
    }
};

std::string Client::getUser(std::string const &pwd) const
{
    std::string user = pwd;
    if (user.find("/home/") != std::string::npos)
    {
        user = user.substr(user.find("/home/") + 6);
        if (user.find("/") != std::string::npos)
            user = user.substr(0, user.find("/"));
    }
    return (user);
}

void Client::setSocketAddress(struct sockaddr_in *addr)
{
    this->clientInfo = *addr;
}

const struct sockaddr_in* Client::getSocketAddress( void ) const
{
    return (&this->clientInfo);
}

char* Client::inet_ntoa(struct in_addr clAddr) const //erevi saya
{
    static char ip[INET_ADDRSTRLEN];
    
    uint32_t ipaddr = htonl(clAddr.s_addr);

    snprintf(ip, sizeof(ip), "%u.%u.%u.%u",
        (ipaddr >> 24) & 0xFF,
        (ipaddr >> 16) & 0xFF,
        (ipaddr >> 8) & 0xFF,
        ipaddr & 0xFF
    );
    std::cout << "~~~~~~" << ip << "~~~~" << ipaddr << std::endl;
    return (ip);
}