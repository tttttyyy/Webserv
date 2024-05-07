#include "HTTPServer.hpp"
#include <map>
#include "utils.hpp"
#include "Cgi.hpp"
#include "EvManager.hpp"
#include "InnerFd.hpp"
#include "Types.hpp"

size_t longestMatch(std::string const &s1, std::string const &s2);

HTTPServer::HTTPServer( void )
{
    this->port = DEFAULT_HTTP_PORT;
    this->ip = DEFAULT_MASK;
    methodsMap["GET"] = &HTTPServer::get;
    methodsMap["POST"] = &HTTPServer::post;
    methodsMap["DELETE"] = &HTTPServer::del;
    // methodsMap["HEAD"] = &HTTPServer::head;
}

HTTPServer::~HTTPServer()
{
    for (size_t i = 0; i < _srvs.size(); i++)
    {
        delete _srvs[i];
    }
    std::map<sock_t, Client *>::iterator it = _clnt.begin();
    while (it != _clnt.end()) {
        delete it->second;
        ++it;
    }
}

void HTTPServer::setPort(std::string const &port)
{
    if (port.size() > 5)
        throw HTTPCoreException("Port: is to long");
    for(size_t i = 0; i < port.size(); i++)
        if (!std::isdigit(port[i]))
            throw HTTPCoreException("Port: Non digit character");
    this->n_port = (uint16_t)std::atoi(port.c_str());
    this->port = port;
}

const char* HTTPServer::getPort( void ) const {return (port.c_str());}

const char* HTTPServer::getIp( void ) const {return (this->ip.c_str());}

// uint16_t HTTPServer::getNPort( void ) const {return (this->n_port);}

// uint32_t HTTPServer::getNIp( void ) const {return (this->n_ip);}

void HTTPServer::setIp(std::string const &ipv)
{
    size_t n = 0;
    size_t k = 0;
    std::string octet;
    
    if (ipv.size() > 15)
        throw HTTPCoreException("The entered IP address does not follow the correct syntax.");
    for(size_t i = 0; i <= ipv.size(); i++)
    {
        if (std::isdigit(ipv[i]))
        {
            n++;
            octet += ipv[i];
        }
        else if (ipv[i] == '.' || i >= ipv.size())
        {
            if (ipv[i] == '.')
                k++;
            if (n > 3 || std::atoi(octet.c_str()) > 255)
                throw HTTPCoreException("The entered IP address does not follow the correct syntax.");
            octet.clear();
            n = 0;
        }
        else
            throw HTTPCoreException("The entered IP address does not follow the correct syntax.");

    }
    if (k != 3)
        throw HTTPCoreException("The entered IP address does not follow the correct syntax.");
    this->ip = ipv;
}

void HTTPServer::push(std::string const &prefix, Location const &locationDirective)
{
    this->_locations.insert(std::make_pair(prefix, locationDirective));
}

void HTTPServer::push__serverName(std::string const &srvName)
{
    std::vector<std::string>::iterator it = std::find(_serverName.begin(), _serverName.end(), srvName);
    if (it == _serverName.end())
        _serverName.push_back(srvName);
}

std::vector<std::string> const &HTTPServer::get_serverNames( void ) const {return (_serverName);}

std::map<std::string, Location> const &HTTPServer::getLocations( void ) const {return (_locations);}

sock_t HTTPServer::getfd( void ) const {return (this->_fd);}

void HTTPServer::up()
{
    const char* givenIp = ip.c_str();
    const char* givenPort = port.c_str();
    Tcp::setup(givenIp, givenPort);
    Tcp::createSocket();
    Tcp::bindSocket();
    Tcp::listenSocket();
    std::cout << givenIp <<  ":" << givenPort << std::endl;
}

void HTTPServer::push(sock_t clFd, Client *clt)
{
    if (clt != NULL)
        _clnt.insert(std::make_pair(clFd, clt));
}

void HTTPServer::push(HTTPServer *srv) {
    if (srv != NULL)
        _srvs.push_back(srv);
}

HTTPServer *HTTPServer::getSubServerByName(std::string const &serverName) {
    for (size_t i = 0; i < _srvs.size(); ++i) {
        std::vector<std::string> &srvNames =  _srvs[i]->_serverName;
        std::vector<std::string>::const_iterator it = std::find(srvNames.begin(),srvNames.end(), serverName);
        if (it != srvNames.end()) {
            return (_srvs[i]);
        }
    }
    return (NULL);
}

bool HTTPServer::exist(sock_t fd) {return (_clnt.find(fd) != _clnt.end());}

bool HTTPServer::operator==(HTTPServer const &sibling) const
{
    if (std::strcmp(this->getIp(), sibling.getIp()) == 0 \
        && std::strcmp(this->getPort(),sibling.getPort()) == 0)
      return (true);  
    return (false);
}

bool HTTPServer::operator==(sock_t fd) const
{
    if (_clnt.find(fd) != _clnt.end()) {
      return (true);
    } 
    return (false);
}

Client* HTTPServer::getClient(sock_t fd)
{
    std::map<sock_t, Client*>::iterator it = _clnt.find(fd);
    if (it != _clnt.end()) {
        return (it->second);
    }
    return (NULL);
}

void HTTPServer::removeClient(sock_t fd)
{
    std::map<sock_t, Client*>::iterator it = _clnt.find(fd);

    if (it != _clnt.end()) {
        delete it->second;
        _clnt.erase(it);
    }
    return ;
}

InnerFd *HTTPServer:: getInnerFd(int fd) {
    std::map<sock_t, Client *>::iterator it = _clnt.begin();
    
    while (it != _clnt.end()) {
        InnerFd *innerFd = it->second->getInnerFd(fd);

        if (innerFd) {
            return(innerFd);
        }
        ++it;
    }
    return (NULL);
}

const Location* HTTPServer::findMatching(std::string const &realPath) const
{
    // std::map<std::string, Location>::const_iterator it = _locations.begin();
    // std::cout << "_locations.size() = " << _locations.size() << std::endl;
    // while (it != _locations.end()) {
    //     std::cout << "_locations.end() = "  << it->first << "$ " << std::endl;
    //     ++it;
    // } 
    // std::cout << "realPath = " << realPath << std::endl;
    std::map<std::string, Location>::const_iterator loc;
    std::map<std::string, Location>::const_iterator match = _locations.end();
    size_t longestMatchSize = 0;
    size_t currentMatch = 0;
    for(loc = _locations.begin(); loc != _locations.end(); loc++)
    {
        currentMatch = longestMatch(loc->first, realPath);
        // std::cout << "currentMatch = " << currentMatch << std::endl;
        if (longestMatchSize < currentMatch && currentMatch != 0)
        {
            match = loc;
            longestMatchSize = currentMatch;
        }
        else if (longestMatchSize == currentMatch && currentMatch != 0)
        {
            if (match->first < loc->first)
                match = loc;
        }
    }
    if (match == _locations.end()) 
    {
        return (NULL);
    }
    // std::cout << "match->first = " << match->first << std::endl;
    return (&match->second);
}
// http://127.0.0.1:3000/directory/images/test.png
size_t longestMatch(std::string const &s1, std::string const &s2)
{
    size_t match = 0;
    size_t i = 0;

    for(; i < s1.size(); i++)
    {
        if (s1[i] != s2[i])
            break;
        match++;
    }
    // std::cout << "match = " << match << std::endl;
    // std::cout << "i = " << i << std::endl;
    // std::cout << "s1.size() = " << s1.size() << std::endl;
    // std::cout << "s2[i - 1] == '/' = " << s2[i - 1] << std::endl;
    // std::cout << "s2[i - 1] == '/' = " << s2[i] << std::endl;
    if (match == 0 || ((s1.size() == i) && (s2.size() == i || s2[i - 1] == '/' || s2[i] == '/'))
        || ((s1.size() - 1 == i) && (s2.size() == i || s2[i - 1] == '/' || s2[i] == '/')))
    {
        return (match);
    }
    return (0);
}

void HTTPServer::get(Client &client) {
    const std::string &path = client.getPath();

    if (access(path.c_str(), R_OK) == 0) {
        std::string fileContent;
        if (client.isCgi() == true) {
            int fd = Cgi::execute(client);
            client.setCgiPipeFd(fd);
        } else {
            if (client.getCurrentLoc().getAutoindex() == true && HTTPRequest::isDir(path)) {
                client.addHeader(std::pair<std::string, std::string>("Content-Type", "text/html"));
                client.buildHeader();
                client.setBody(directory_listing(path, client.getDisplayPath()));
            } else if (HTTPRequest::isDir(path)) {
                throw ResponseError(404, "not found");
            } else {
                int fd = open(path.c_str(), O_RDONLY);
                if (fd == -1) {
                    throw ResponseError(500, "Internal Server Error");
                }
                EvManager::addEvent(fd, EvManager::read, EvManager::inner);
                EvManager::addEvent(fd, EvManager::write, EvManager::inner);
                client.addInnerFd(new InnerFd(fd, client, client.getResponseBody(),  EvManager::read));
            }

            std::map<std::string, std::string>::iterator mime = Types::MimeTypes.find(client.getExtension());
            std::string mimeType;
            if (mime != Types::MimeTypes.end())
                mimeType = mime->second;
            else
                mimeType = "text/plain";
            client.addHeader(std::pair<std::string, std::string>("Content-Type", mimeType));
        }
    } else {
        throw ResponseError(404, "not found");
    }
}

void HTTPServer::post(Client &client) {
    // client.addHeader(std::pair<std::string, std::string>("content-type", "text/html"));
    if (client.isCgi() == true) {
        int fd = Cgi::execute(client);
        client.setCgiPipeFd(fd);
    } else {
        throw ResponseError(501, "Not Implemented");
    }
}

void HTTPServer::del(Client &client) {
    if (std::remove(client.getPath().c_str()) == -1) {
        throw ResponseError(404, "not found");
    };
    client.getResponseBody() = "ok";
    client.addHeader(std::pair<std::string, std::string>("Content-Length", my_to_string(client.getResponseBody().size())));
    client.buildHeader();
    client.isResponseReady() = true;
}

// void HTTPServer::head(Client &client) {
//     std::cout << client.getMethod() << " " <<  client.getPath() << std::endl;
//     HTTPServer::get(client);
//     client.setBody("");
//     std::cout << client.getResponseLine() << client.getResponseHeader() << client.getResponseBody();
// };

void HTTPServer::processing(Client &client)
{
    // std::cout << client.getMethod() << " " << client.getPath() << std::endl;
    // std::cout << "cgi status" << " " << client.isCgi() << std::endl;
    // client.showHeaders();
    std::map<std::string, void (HTTPServer::*)(Client&)>::iterator function = methodsMap.find(client.getMethod());
    if (function != methodsMap.end() && (client.getCurrentLoc().findMethod(client.getMethod()) != NULL))
    {
       (this->*(function->second))(client);
    } else {
        // std::cout << "Method Not Allowed\n\n\n\n\n\n\n\n";
        throw ResponseError(405, "Method Not Allowed");
    }
}

std::string	HTTPServer::directory_listing(const std::string &path, std::string displayPath)
{
	DIR					*opened_dir;
	dirent				*dir_struct;
	std::string			table;
	std::string			name;
	struct stat			buf;
	struct tm			*timeinfo;
	char				time_buf[100];
	std::string 		relPath;
	
	if (path != "." && path != ".." && path[0] != '/' && (path[0] != '.' && path[1] != '/') && (path[0] != '.' && path[1] != '.' && path[2] != '/')) {
        relPath = "./" + path + "/";
    } else
		relPath = path + "/";

	opened_dir = opendir(relPath.c_str());
	
    if (opened_dir == NULL)
        throw std::logic_error (strerror(errno));

	table += "<!DOCTYPE html><html><head><title>";
	table += "Index of ";
	table += displayPath;
	table += "</title>";
	table += "<style>";
	table += ".box>* {flex: 33.33%;}";
	table += ".box {display: flex; flex-wrap: wrap; width: 75%;}";
	table += "</style></head>";
	table += "<body><h1>";
	table += "Index of ";
	table += displayPath;
	table += "</h1><hr><pre class=\"box\">";

	dir_struct = readdir(opened_dir);
	while ((dir_struct = readdir(opened_dir)) != NULL)
	{
		name = dir_struct->d_name;
		if (name != ".")
		{
			table += "<a href=\"";
            if (displayPath.empty() == false && displayPath[displayPath.size() - 1] != '/')
            {
    			displayPath +=  "/";
            }
			table += displayPath + name;
			table += "\">";
			table += name;
			table += "</a>";
			if (stat((relPath + name).c_str(), &buf) == 0)
			{
				table += "<span>";
				timeinfo = localtime(&(buf.st_mtime));
				strftime(time_buf, 100, "%d-%b-%Y %H:%S", timeinfo);
				table += time_buf;
				table += "</span><span>";
				if (dir_struct->d_type == DT_DIR)
					table += "-";
				table += "</span>";
			}
			table += "<br>";
		}
	}
	table += "</pre><hr></body></html>";
	closedir(opened_dir);
    return table;
}