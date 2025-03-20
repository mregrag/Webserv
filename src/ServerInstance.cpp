#include "../include/webserver.hpp"

ServerInstance::ServerInstance(const ServerConfig& config)
{
    _config = config;
    setupSocket();
}

ServerInstance::ServerInstance(){}

ServerInstance::~ServerInstance()
{
    close(_listenFd);
}

int ServerInstance::getListenFd() const
{
    return _listenFd;
}

const ServerConfig& ServerInstance::getConfig() const
{
    return _config;
}

void    ServerInstance::setConfig(const ServerConfig& config)
{
    _config = config;
    setupSocket();
}

void ServerInstance::setupSocket()
{
    _listenFd = socket(AF_INET, SOCK_STREAM, 0);
    if (_listenFd == -1)
        throw(std::runtime_error("error socket opening!"));

    int flags = fcntl(_listenFd, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error("fcntl get failed");

    if (fcntl(_listenFd, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl set non-blocking failed");

    struct sockaddr_in   server_addr;
    memset(&server_addr, 0, sizeof (server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(_config.getPort());
    if (inet_pton(AF_INET, _config.getHost().c_str(), &server_addr.sin_addr) < 0)
        throw(std::runtime_error("Invalid IP address"));

    if (bind(_listenFd, (sockaddr *)&server_addr, sizeof (server_addr)) < 0)
        throw(std::runtime_error("error binding!"));

    if (listen(_listenFd, MAX_EVENTS) < 0)
        throw(std::runtime_error("error lisning!"));
}
