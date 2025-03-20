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
    EpollManager                    epollInstance;
    std::vector<struct epoll_event> events;

    for (std::vector<ServerInstance>::iterator it = _servers.begin()
            ; it != _servers.end(); it++)
    {
        epollInstance.addFd((*it).getListenFd(), EPOLLIN);
    }

    while (true)
    {
        int num_event = epollInstance.wait(events, -1);
        for (int i = 0; i < num_event; i++)
        {
            int eventFd = events.data()[i].data.fd;
            for (std::vector<ServerInstance>::iterator it = _servers.begin()
                ; it != _servers.end(); it++)
            {
                if (eventFd == (*it).getListenFd()) 
                    handleNewConnection((*it).getListenFd());
                else
                    handleClientEvent(eventFd);
            }
        }
    }
}

void    ServerManager::handleNewConnection(int serverFd)
{
    std::cout << "handle New Connection" << std::endl;
    (void)serverFd;
}

void    ServerManager::handleClientEvent(int clientFd)
{
    std::cout << "handle Client Event" << std::endl;
    (void) clientFd;
}