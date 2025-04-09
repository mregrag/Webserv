#ifndef SERVER_MANAGER_HPP
#define SERVER_MANAGER_HPP

#include "webserver.hpp"

// class Request;
// class Response;
// class Config;
// class EpollManager;

class ServerManager
{
public:
    ServerManager(const std::vector<ServerConfig>& configs);
    ~ServerManager();

    void    run();  // Main event loop handling epoll events
    void    add_client(int clientFd, int serverFd);

private:
    void    initServers(const std::vector<ServerConfig> &configs);  // Sets up listening sockets for all servers
    void    handleNewConnection(int serverFd,  ServerInstance &server, EpollManager& epollInstance);  // Accepts new clients
    void    handleClientEvent(int clientFd);  // Reads/Writes data for clients

    std::vector<ServerInstance> _servers; // All running server instances
    std::map<int, ClientSession*> _clients; // Active client connections (fd -> ClientSession)
    EpollManager _epollInstance; // Epoll instance to manage events
};

#endif