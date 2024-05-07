#include "Cgi.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <ctime>

std::map<std::string, std::string> Cgi::_env;

int Cgi::execute(Client &client) {
    signal(SIGPIPE, SIG_IGN); // TODO
    char *argv[3];
    const std::string &argv1 = client.getCurrentLoc().getCgi(client.getExtension()).second;
    argv[0] = const_cast<char *>(argv1.c_str());
    const std::string &argv2 =  client.getPath();
    argv[1] = const_cast<char *>(argv2.c_str());
    argv[2] = NULL;
    int pipe_from_child[2];

    if (pipe(pipe_from_child) == -1
            || fcntl(pipe_from_child[0], F_SETFL, O_NONBLOCK, O_CLOEXEC) == -1) {
        throw ResponseError(500, "Internal Server Error");
    }
    int pid = fork();

    if (pid == -1) {
        throw ResponseError(500, "Internal Server Error");
    }
    // std::ofstream osf("log.log");
    // osf << client.getRequestBody();
    if (pid == 0) {
        if (client.getMethod() == "POST") {
            int fd = open(client.getTmpToChild().c_str(),  O_RDONLY);
            dup2(fd, 0);
            close(fd);
        }
        char **envp = Cgi::initEnv(client);

        dup2(pipe_from_child[1], 1);
        close(pipe_from_child[0]);
        close(pipe_from_child[1]);

        int res = execve(argv[0], argv, envp);
        perror("execve: ");
        exit(res);
    }
    close(pipe_from_child[1]);
    client.setCgiPID(pid);
    client.setCgiStartTime();
    return (pipe_from_child[0]);
};

char **Cgi::initEnv(Client const &client)
{
    char *pwd;
    const ServerCore &srv = client.getCurrentLoc();
    char *clientIp = client.inet_ntoa(client.getSocketAddress()->sin_addr);

    // client.showHeaders();
    pwd = getcwd(NULL, 0);
    _env["AUTH_TYPE"] = "Basic";
    std::string length = client.findInMap("Content-Length");
    if (length.empty() == false) {
        _env["CONTENT_LENGTH"] = length;
    }
    _env["CONTENT_TYPE"] = client.findInMap("Content-Type");
    // _env["TRANSFER-ENCODING"] = client.findInMap("Transfer-Encoding");
    _env["GATEWAY_INTERFACE"] = "CGI/1.1";
    _env["PATH_INFO"] = client.getPath();
    _env["PATH_TRANSLATED"] = pwd + std::string("/") + client.getPath();
    _env["QUERY_STRING"] = client.getQueryString();
    _env["REMOTE_ADDR"] = clientIp;
    _env["REMOTE_HOST"] = client.findInMap("Host");
    _env["REMOTE_USER"] = client.getUser(pwd);
    _env["REQUEST_METHOD"] = client.getMethod();
    _env["SCRIPT_NAME"] = client.getPath();
    _env["SCRIPT_FILENAME"] = pwd + std::string("/") + client.getPath();
    _env["SERVER_NAME"] = "webserv";
    _env["SERVER_PORT"] = client.getServerPort();
    _env["SERVER_PROTOCOL"] = "HTTP/1.1";
    _env["SERVER_SOFTWARE"] = "Webserv";
    _env["SERVER_WRITE_PATH"] = srv.getUploadDir();
    _env["UPLOAD_DIR"] = srv.getUploadDir();
    _env["LC_CTYPE"] = "C.UTF-8";
    _env["REDIRECT_STATUS"] = "true";
	free(pwd);

    char **envp = new char *[_env.size() + 1];

	int i = 0;
    // std::ofstream ofs("env.log");
	for (std::map<std::string, std::string>::iterator it = _env.begin(); it != _env.end(); ++it)
	{
		envp[i++] = strdup((it->first + "=" + it->second).c_str());
        // ofs << envp[i - 1] << std::endl;;
	}

	envp[i] = NULL;

	return envp;
};