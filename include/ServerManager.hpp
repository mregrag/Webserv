#ifndef SERVERMANAGER_HPP
#define SERVERMANAGER_HPP

#include "ServerConfig.hpp"
#include "../include/Logger.hpp"
#include <vector>
#include <map>
#include <string>
#include <ctime>
#include <cstring>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Client.hpp"
#include "Epoll.hpp"
#include <netinet/in.h>
#include <sys/epoll.h>
#include <wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <algorithm>


class ServerManager
{
private:
	bool                                _running;
	std::vector<ServerConfig>           _servers;
	std::vector<int>                    _serverFds;
	std::map<int, Client*>              _clients;
	Epoll                        _epoll;

public:
	ServerManager(const std::vector<ServerConfig>& servers);
	~ServerManager();

	bool init();
	void run();
	void stop();

private:
	int createAndBindSocket(const std::string& host, uint16_t port);

	void handleEvent(const epoll_event& event);
	void acceptClient(int fd);

	void checkTimeouts();
	void cleanupClient(int fd);
};

#endif
