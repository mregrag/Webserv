#include "../include/CGIHandler.hpp"
#include <fcntl.h>
#include <unistd.h>

CGI::CGI(HTTPRequest &request)
    : _pid(-1), _requestBodyFileFd(-1), _tmpFileFd(-1), _request(request),
    _args(NULL), _env(NULL)
{
    LOG_INFO("Cgi constructed");
}

CGI::~CGI(void)
{
    if (_args) {
        for (int i = 0; _args[i]; ++i)
            free(_args[i]);
        delete[] _args;
    }

    if (_env) {
        for (int i = 0; _env[i]; ++i)
            free(_env[i]);
        delete[] _env;
    }

    // request body file fd not closed

    LOG_INFO("Cgi destroyed");
}

void	CGI::_initTmpFile(void)
{
    std::string	name;
    int	i = 0;
    std::string	res;

    name = "/tmp/webserv_";
    do
    {
        res = name + Utils::toString(i);
        i++;
    } while (access(res.c_str(), F_OK) == 0);
    _tmpFileFd = open(res.c_str(), O_CREAT | O_TRUNC | O_RDWR, 0777); // for writing only
    if (_tmpFileFd == -1)
        LOG_ERROR("tmp file: " + res + " not created");
    else
        LOG_DEBUG("tmp file: " + res + " created successfully");

    _request.filename = res;
}

void	CGI::_initEnv(void)
{
    _env = new char*[6];
    
    _env[0] = strdup(std::string("CONTENT_TYPE=" + _request.getHeaderValue("Content-Type")).c_str());
    _env[1] = strdup(("CONTENT_LENGTH=" + _request.getHeaderValue("Content-Length")).c_str());
    _env[2] = strdup(("QUERY_STRING=" + _request.getQuery()).c_str());
    _env[3] = strdup(("REQUEST_URI=" + _request.getUri()).c_str());
    _env[4] = strdup(("REQUEST_METHOD=" + _request.getMethod()).c_str());
    _env[5] = NULL;

    LOG_DEBUG("Init args finished");
}

void	CGI::_initArgs(void)
{
    // example /usr/bin/python3 finalRequestedResource NULL
    _args = new char*[3];
    
    _args[0] = strdup(_request.getLocation()->getCgiPath().c_str());
    _args[1] = strdup(_request.getResource().c_str());
    _args[2] = NULL;

    LOG_DEBUG("Init env finished");
}

void	CGI::init(void)
{
    LOG_DEBUG("Initing cgi");

    _initTmpFile();
    _initArgs();
    _initEnv();
}

#include <sys/wait.h>

void	CGI::execute(void)
{
    _pid = fork();
    if (_pid == -1)
        return LOG_ERROR("fork failed");

    if (_pid != 0)
    {
        LOG_INFO("================== Executing cgi with ==================");
        // std::cout << *this << std::endl;
        LOG_INFO("========================================================");
    }

    if (_pid == 0)
    {
        // read from request body file
        if (_requestBodyFileFd != -1)
        {
            if (dup2(_requestBodyFileFd, 0) == -1)
                LOG_ERROR("dup2 1 failed");
        }

        // write to tmp file
        if (dup2(_tmpFileFd, 1) == -1)
            LOG_ERROR("dup2 2 failed");

        close(_tmpFileFd);

        execve(_args[0], _args, _env);
        LOG_ERROR("execve failed");
        exit(EXIT_FAILURE);
    }

    usleep(500);

    _request.pid = _pid;
}

std::ostream &operator<<(std::ostream &os, const CGI &cgi)
{
    os << "CGI Object:\n"
       << "  PID: " << cgi._pid << "\n"
       << "  Request Body File FD: " << cgi._requestBodyFileFd << "\n"
       << "  Temporary File FD: " << cgi._tmpFileFd << "\n"
       << "  HTTP Request: " << &cgi._request << "\n"
       << "  Args: ";
    
    if (cgi._args)
    {
        for (int i = 0; cgi._args[i]; ++i)
            os << cgi._args[i] << (cgi._args[i + 1] ? ", " : "");
    }
    else
        os << "null";

    os << "\n  Env: ";
    if (cgi._env)
    {
        for (int i = 0; cgi._env[i]; ++i)
            os << cgi._env[i] << (cgi._env[i + 1] ? ", " : "");
    }
    else
        os << "null";

    return os;
}
