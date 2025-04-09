#include "../include/webserver.hpp"

ServerManager::ServerManager(const std::vector<ServerConfig> &configs)
{
    initServers(configs);
}

void ServerManager::initServers(const std::vector<ServerConfig>& serverConfigs)
{
    _servers.resize(serverConfigs.size());
    int i = -1;
    while (++i < _servers.size())
        _servers[i].setConfig(serverConfigs[i]);
}


ServerManager::~ServerManager()
{
    //TODO    clean up the client stuf here;

    //close all the server sockets
    for (std::vector<ServerInstance>::iterator it = _servers.begin()
        ; it != _servers.end(); it++)
    {
        close((*it).getListenFd());
    }
}

void    ServerManager::run()
{
    std::vector<struct epoll_event> events;

    for (std::vector<ServerInstance>::iterator it = _servers.begin()
            ; it != _servers.end(); it++)
        _epollInstance.addFd((*it).getListenFd(), EPOLLIN);

    while (true)
    {
        int num_event = _epollInstance.wait(events, -1);
        for (int i = 0; i < num_event; i++)
        {
            int eventFd = events.data()[i].data.fd;
            for (std::vector<ServerInstance>::iterator it = _servers.begin()
                ; it != _servers.end(); it++)
            {
                if (eventFd == (*it).getListenFd()) // new connection
                    handleNewConnection(eventFd, *it, _epollInstance);
                else
                    handleClientEvent(eventFd);
            }
        }
    }
}



void    ServerManager::handleNewConnection(int serverFd,  ServerInstance &server,  EpollManager& epollInstance)
{
    struct sockaddr_in  client_addr;
    socklen_t           addr_size = sizeof(client_addr);

    addr_size ;
    int client_sock = accept(server.getListenFd(), (sockaddr*)&client_addr, &addr_size);
    if (client_sock == -1)
        throw(std::runtime_error("error: accepting conection!"));
    
    int flags = fcntl(client_sock, F_GETFL, 0);
    fcntl(client_sock, F_SETFL, flags | O_NONBLOCK);
    
    std::cout << "client_sock = " << client_sock << std::endl;
    epollInstance.addFd(client_sock, EPOLLIN | EPOLLET);
    ClientSession   client(serverFd);
    _clients[client_sock] = &client;
}

void    ServerManager::handleClientEvent(int clientFd)
{
    _clients[clientFd]->handleRequest();
}