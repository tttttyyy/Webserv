#include "ServerManager.hpp"
#include "EvManager.hpp"
#include "utils.hpp"
#include "InnerFd.hpp"

bool ServerManager::newClient(int fd) {
    for (size_t i = 0; i < this->size(); ++i) {
        if ((*this)[i]->getfd() == fd) {
            sock_t clientFd = (*this)[i]->accept();
            if (clientFd == -1) {
                throw std::runtime_error(std::string("accept: ") + strerror(errno));
            }
            fcntl(clientFd, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
            Client *client = new Client(clientFd, (*this)[i]->getfd(), *(*this)[i]);
            client->setSocketAddress((*this)[i]->getClientAddress());
            EvManager::addEvent(clientFd, EvManager::read, EvManager::client);
            (*this)[i]->push(clientFd, client);
            return (true);
        }
    }
    return (false);
}

void manageHeader(std::string &str, Client &client) {
    size_t pos = str.find("\r\n\r\n");
    if (pos != std::string::npos) {
        std::string header = str.substr(0, pos);
        str.erase(0, pos + strlen("\r\n\r\n"));
        std::stringstream sheader(header);
        std::string line;        
        while (std::getline(sheader, line, '\n'))
        {
            std::stringstream sline(line);
            std::string key;
            std::string value;
            std::getline(sline, key, ':');
            std::getline(sline, value);
            key = HTTPRequest::trim(key);
            value = HTTPRequest::trim(value);
            
            if (key.empty() == false && value.empty() == false) {
                client.addHeader(std::pair<std::string, std::string>(key, value));   
            }
        }
    }
}

bool checkInnerFd(HTTPServer &srv, int fd) {
    InnerFd *innerFd = srv.getInnerFd(fd);
    if (innerFd) {
        Client &client = innerFd->_client;
        if (innerFd->_flag ==  EvManager::read) {
            if (client.getResponseBody().empty()) {
                EvManager::addEvent(innerFd->_fd, EvManager::write, EvManager::inner);
            }
            if (readFromFd(innerFd->_fd, innerFd->_str) == true) {
                // std::cout << "readFromFd\n";
                if (client.isCgi() == true) {
                    manageHeader(innerFd->_str, client);
                }
                client.getResponseBody() = innerFd->_str;
                client.addHeader(std::pair<std::string, std::string>("Content-Length", my_to_string(client.getResponseBody().size())));
                client.buildHeader();
                client.isResponseReady() = true;
                EvManager::delEvent(innerFd->_fd, EvManager::read);
                EvManager::delEvent(innerFd->_fd, EvManager::write);
                close(innerFd->_fd);
                client.removeInnerFd(innerFd->_fd);
            };
        } else if (innerFd->_flag == EvManager::write) {
            int res = writeInFd(innerFd->_fd, innerFd->_str);
            if ((client.isBodyReady() == true && innerFd->_str.empty() == true) || res == -1) {
                EvManager::delEvent(innerFd->_fd, EvManager::read);
                EvManager::delEvent(innerFd->_fd, EvManager::write);
                close(innerFd->_fd);
                client.removeInnerFd(innerFd->_fd);
                if (res == -1) {
                    throw ResponseError(500, "Internal Server Error", client);
                }
            }
        }
        return (true);
    }
    return (false);
}

std::ofstream ofsLog("ofsLog.log");

void ServerManager::start() {
    EvManager::start();

    for (size_t i = 0; i < this->size(); ++i) {
        EvManager::addEvent((*this)[i]->getfd(), EvManager::read, EvManager::server);
    }

    while(true) {
        Client *client = NULL;
        std::pair<EvManager::Flag, int> event;
        
        try
        {
            event = EvManager::listen();
            // std::ofstream ofsFd("fd.log");
            // ofsFd << "event = " << event.first << std::endl;
            // ofsFd << "second = " << event.second << std::endl;
            // std::cout << "event = " << event.first << std::endl;
            // std::cout << "second = " << event.second << std::endl;
            if (newClient(event.second)) {
                continue ;
            }
            for (size_t i = 0; i < this->size(); ++i) {
                if (checkInnerFd(*(*this)[i], event.second) == true) {
                    continue ;
                }
            }
            for (size_t i = 0; i < this->size(); ++i) {
                client = (*this)[i]->getClient(event.second);
                if (client) {
                    break;
                }
            }
            if (client == NULL) {
                continue ;
            }
            if (event.first == EvManager::eof) {
                closeConnetcion(*client);
                continue ;
            } else if (client->isRequestReady() == false && event.first == EvManager::read) {
                if (client->getHttpRequest().empty()) {
                    EvManager::addEvent(client->getFd(), EvManager::write, EvManager::client);
                }
                if (client->receiveRequest() == -1) {
                    closeConnetcion(*client);
                    continue ;
                }
                if (client->isRequestReady() == true) {
                    ofsLog << "request received " << std::endl;
                    EvManager::delEvent(client->getFd(), EvManager::read);
                }
                if (/* client->isInProgress() &&  */client->isStarted() == false && client->isRequestReady() == true) {
                    client->setStartStatus(true);
                    generateResponse(*client);
                }
            } else if ((client->isRequestReady() == true || client->isErrorResponse() == true)
                    && client->isResponseReady() && event.first == EvManager::write) {
                int res = client->sendResponse();
                if (res == -1 || client->findInMap("Connection") == "close") {
                    ofsLog << "response sent" << std::endl;
                    closeConnetcion(*client);
                    continue ;
                } else if (res == true) {
                    ofsLog << "response sent" << std::endl;
                    Client *newClient = new Client(client->getFd(), client->getServerFd(), client->getDefaultSrv());
                    client->getDefaultSrv().removeClient(client->getFd());
                    newClient->getDefaultSrv().push(newClient->getFd(), newClient);
                    EvManager::delEvent(newClient->getFd(), EvManager::write);
                    EvManager::addEvent(newClient->getFd(), EvManager::read, EvManager::client);
                    continue ;
                }
            } else if (client->isCgi() == true) {
                client->checkCgi();
            }
        }
        catch(ResponseError& e)
        {
            Client *tmpClient = e.getClient();
            if (tmpClient == NULL)
               tmpClient = client;
            if (tmpClient && tmpClient->isErrorResponse() == false)
                generateErrorResponse(e, *tmpClient);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
        }
    }
};


void ServerManager::generateErrorResponse(const ResponseError& e, Client &client) {
    std::string response;
    std::string resBody;
    // std::cout << "generateErrorResponse\n";
    if (e.getStatusCode() == 301) {
        client.addHeader(std::make_pair("Location", client.getRedirectPath()));
    } else if (client.getMethod() != "HEAD") {
        try
        {
            resBody = fileToString(client.getCurrentLoc().getErrPage(e.getStatusCode()));
        }
        catch(...)
        {
            resBody += "<html>";
            resBody += "<head><title>" + my_to_string(e.getStatusCode()) + " " + e.what() + "</title></head>";
            resBody += "<body>";
            resBody += "<center><h1>" + my_to_string(e.getStatusCode()) + " " + e.what() + "</h1></center><hr>";
            resBody += "</body>";
            resBody += "</html>";
        }
    }

    response = HTTP_VERSION;
    response += " " + my_to_string(e.getStatusCode());
    response += " ";
    response += e.what();
    response += "\r\n";
    client.setResponseLine(response);
    client.addHeader(std::pair<std::string, std::string>("Content-Type", "text/html"));
    client.addHeader(std::pair<std::string, std::string>("Content-Length", my_to_string(resBody.size())));
    client.buildHeader();
    client.setBody(resBody);
    client.setIsErrorResponse(true);
}

void ServerManager::generateResponse(Client &client) {
    std::string response;

    response = client.getVersion();
    response += " " + std::string("200") + " ";
    response += SUCCSSES_STATUS;
    response += "\r\n";
    client.setResponseLine(response);
    try
    {
       client.getSrv().processing(client);
    }
    catch(const ResponseError& e)
    {
        generateErrorResponse(e, client);
    }
}

bool ServerManager::closeConnetcion(Client &client) {
    int fd = client.getFd();
    EvManager::delEvent(fd, EvManager::read);
    EvManager::delEvent(fd, EvManager::write);
    close(fd);
    HTTPServer &srv = client.getDefaultSrv();
    srv.removeClient(fd);
    return (true);
};

ServerManager::ServerManager(const char *configfile)
{
    Parser parser(configfile);
    parser.start(*this);
}

ServerManager::~ServerManager()
{
    for (size_t i = 0; i < this->size(); i++) {
        delete (*this)[i];
    }
}


void ServerManager::clearInstances( void )
{
    for (size_t i = 0; i < this->size(); i++) {
        delete (*this)[i];
    }
    this->clear();
}

/*************************************************************
Finding correct HTTPServer funtions based on ::ServerManager::
**************************************************************/
HTTPServer *ServerManager::getServerBySocket(sock_t fd)
{
    for(size_t i = 0; i < this->size(); i++)
    {
        if (fd == (*this)[i]->getfd())
            return ((*this)[i]);
    }
    return (NULL);
}

HTTPServer *ServerManager::getServerByClientSocket(sock_t fd)
{
    for(size_t i = 0; i < this->size(); i++)
    {
        if ((*this)[i]->getClient(fd))
            return ((*this)[i]);
    }
    throw std::logic_error("getServerByClientSocket");
    return (NULL);
}

int ServerManager::used(HTTPServer &srv) const
{
    for(size_t i = 0; i < this->size(); i++)
        if (std::strcmp((*this)[i]->getPort(), srv.getPort()) == 0)
        {
            return (i);
        }
    return (-1);
}