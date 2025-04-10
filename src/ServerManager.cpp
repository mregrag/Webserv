#include "../include/webserver.hpp"
#include "../include/ServerManager.hpp"

#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <iostream>

ServerManager::ServerManager() 
{
	LOG_INFO("ServerManager initialized");
}

ServerManager::~ServerManager() 
{
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
		close(it->first);
		LOG_DEBUG("Closed client connection (fd: " + toString(it->first) + ")");
	}
	for (size_t i = 0; i < _servers.size(); ++i) 
	{
		close(_servers[i].getFd());
		LOG_DEBUG("Closed server socket (fd: " + toString(_servers[i].getFd()) + ")");
	}
	LOG_INFO("ServerManager shutdown complete");
}

void ServerManager::setupServers(const std::vector<ServerConfig>& servers) 
{
	_servers = servers;
	LOG_INFO("Initializing servers");

	for (size_t i = 0; i < _servers.size(); ++i) 
	{
		_servers[i].setupServer();
		int fd = _servers[i].getFd();

		if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) 
		{
			LOG_ERROR("Failed to set non-blocking mode for fd: " + toString(fd));
			throw std::runtime_error("fcntl failed");
		}

		_epollManager.addFd(fd, EPOLLIN | EPOLLET);
		_server_map[fd] = &_servers[i];

		LOG_INFO("Server created: " + _servers[i].getServerName() + " on " + _servers[i].getHost() + ":" + toString(_servers[i].getPort()) + " (fd: " + toString(fd) + ")");
	}
}

void ServerManager::run() 
{
	std::vector<struct epoll_event> events(MAX_EVENTS);

	while (true) {
		int count = _epollManager.wait(events, -1);
		LOG_DEBUG("Processing " + toString(count) + " events");

		for (int i = 0; i < count; ++i) 
		{
			try 
			{
				handleEvent(events[i]);
			}
			catch (const std::exception& e) 
			{
				LOG_ERROR("Error handling event: " + std::string(e.what()));
				int fd = events[i].data.fd;
				if (_clients.find(fd) != _clients.end())
					closeConnection(fd);
			}
		}
	}
}

void ServerManager::handleEvent(struct epoll_event& event) 
{
	int fd = event.data.fd;

	if (_server_map.find(fd) != _server_map.end()) 
		acceptConnection(*_server_map[fd]);
	else if (event.events & EPOLLIN) 
		readRequest(fd);
	else if (event.events & EPOLLOUT) 
		sendResponse(fd);
	else 
		closeConnection(fd);
}

void ServerManager::acceptConnection(ServerConfig& server) 
{
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	memset(&client_addr, 0, len);

	int client_fd = accept(server.getFd(), (struct sockaddr*)&client_addr, &len);
	if (client_fd == -1) 
		throw std::runtime_error("Accept failed");

	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1) 
		throw std::runtime_error("Failed to set client socket non-blocking");

	_epollManager.addFd(client_fd, EPOLLIN | EPOLLET | EPOLLRDHUP);

	Client client;
	client.setSocket(client_fd);
	client.setAddress(client_addr);
	client.setServer(server);

	_clients[client_fd] = client;

	char ip_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, INET_ADDRSTRLEN);
	LOG_INFO("Accepted new connection from " + std::string(ip_str) + " on server " + server.getServerName() + " (fd: " + toString(client_fd) + ")");
}

void ServerManager::readRequest(int client_fd) 
{
	char buffer[4096];
	ssize_t n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

	std::cout << buffer << std::endl;

	if (n <= 0) 
	{
		closeConnection(client_fd);
		return;
	}

	buffer[n] = '\0';
	// Parse the HTTP request using the Client's HttpRequest object.
	_clients[client_fd].request.parse();
	// Build the response (e.g., "Hello, World!") using the Client method.
	_clients[client_fd].buildResponse();

	// Modify the client fd to be ready for writing.
	_epollManager.modifyFd(client_fd, EPOLLOUT | EPOLLET);
}

void ServerManager::sendResponse(int client_fd) 
{
	std::string response = _clients[client_fd].response.getResponse();
	send(client_fd, response.c_str(), response.length(), 0);
	closeConnection(client_fd);
}

void ServerManager::closeConnection(int client_fd) 
{
	_epollManager.removeFd(client_fd);
	close(client_fd);
	_clients.erase(client_fd);
	LOG_DEBUG("Closed connection (fd: " + toString(client_fd) + ")");
}

