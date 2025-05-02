#include "../include/webserver.hpp"
#include "../include/ServerManager.hpp"
#include "../include/LocationConfig.hpp"
#include "../include/Utils.hpp"

// ServerManager.cpp
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <ctime>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

ServerManager::ServerManager() 
{
	LOG_INFO("ServerManager initialized");
}

ServerManager::~ServerManager() 
{
	cleanup();
	LOG_INFO("ServerManager shutdown complete");
}

void ServerManager::setupServers(const std::vector<ServerConfig>& servers) 
{
	_servers = servers;
	LOG_INFO("Initializing " + Utils::toString(_servers.size()) + " servers");

	// Configure each server and its sockets
	for (size_t i = 0; i < _servers.size(); ++i) 
	{
		ServerConfig& server = _servers[i];
		server.setupServer();
		const std::vector<int>& server_fds = server.getFds();

		for (size_t j = 0; j < server_fds.size(); ++j) 
			registerServerSocket(server_fds[j], &server);
	}
}

void ServerManager::run() 
{
	std::vector<struct epoll_event> events(MAX_EVENTS);
	time_t last_timeout_check = time(NULL);
	LOG_INFO("Server started and running");

	while (true) 
	{
		// Wait for epoll events (1-second timeout)
		int num_events = _epollManager.wait(events, 1000);
		time_t now = time(NULL);

		if (now - last_timeout_check >= TIMEOUT_CHECK_INTERVAL) 
		{
			checkClientTimeouts();
			last_timeout_check = now;
		}

		for (int i = 0; i < num_events; ++i) 
			handleEvent(events[i]);
	}
}

void ServerManager::configureSocket(int fd) 
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) 
		throw std::runtime_error("Failed to set socket non-blocking: " + std::string(strerror(errno)));
}

void ServerManager::registerServerSocket(int server_fd, ServerConfig* server) 
{
	configureSocket(server_fd);
	_epollManager.addFd(server_fd, EPOLLIN | EPOLLET);
	_serverMap[server_fd] = server;
	/*LOG_DEBUG("Registered server socket (fd: " + Utils::toString(server_fd) + ")");*/
}

void ServerManager::cleanup() 
{
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) 
		closeClientConnection(it->first);

	_clients.clear();

	// Close all server sockets
	for (size_t i = 0; i < _servers.size(); ++i) 
	{
		const std::vector<int>& fds = _servers[i].getFds();
		for (size_t j = 0; j < fds.size(); ++j) 
		{
			close(fds[j]);
			/*LOG_DEBUG("Closed server socket (fd: " + Utils::toString(fds[j]) + ")");*/
		}
	}
	_servers.clear();
	_serverMap.clear();
}

void ServerManager::handleEvent(const struct epoll_event& event) 
{
	int fd = event.data.fd;

	if (_serverMap.find(fd) != _serverMap.end()) 
		return(acceptNewConnection(fd));

	if (_clients.find(fd) != _clients.end()) 
		return(processClientEvent(fd, event.events));

	/*LOG_ERROR("Unknown file descriptor: " + Utils::toString(fd));*/
	close(fd);
}

void ServerManager::acceptNewConnection(int server_fd) 
{
	ServerConfig* server = _serverMap[server_fd];
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	memset(&client_addr, 0, client_len);

	// Accept new client connection
	int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd == -1) 
	{
		/*LOG_ERROR("Accept failed on fd " + Utils::toString(server_fd) + ": " + std::string(strerror(errno)));*/
		return;
	}

	try 
	{
		// Configure and register client socket
		configureSocket(client_fd);
		_epollManager.addFd(client_fd, EPOLLIN | EPOLLET | EPOLLRDHUP);
		_clients[client_fd] = new Client(client_fd, server);
		/*LOG_DEBUG("Accepted new client (fd: " + Utils::toString(client_fd) + ")");*/
	} 
	catch (const std::exception& e) 
	{
		LOG_ERROR("Failed to setup client (fd: " + Utils::toString(client_fd) + "): " + e.what());
		close(client_fd);
	}
}
void ServerManager::processClientEvent(int client_fd, uint32_t events) 
{
	Client* client = _clients[client_fd];
	client->updateActivity();

	/*LOG_DEBUG("Processing client event on fd " + Utils::toString(client_fd) + ", events: 0x" + Utils::toString(events));*/

	try {
		// Handle errors or connection closure
		if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) 
			throw std::runtime_error("Client connection closed");

		// Handle incoming data
		if (events & EPOLLIN) 
		{
			client->handleRead();

			// Switch to write mode if request is complete
			if (client->getRequest()->getState() == HTTPRequest::FINISH) 
			{
				/*LOG_DEBUG("Request complete, switching to EPOLLOUT for client " + Utils::toString(client_fd));*/
				_epollManager.modifyFd(client_fd, EPOLLOUT | EPOLLET | EPOLLRDHUP);
			}
		}

		// Handle outgoing response
		if (events & EPOLLOUT)
		{
			client->handleWrite();
			if (client->getResponse()->getState() == HTTPResponse::FINISH) 
			{
				LOG_DEBUG("Response complete, switching to EPOLLIN for client " + Utils::toString(client_fd));
				if (client->getRequest()->keepAlive()) 
				{
					client->reset();
					_epollManager.modifyFd(client_fd, EPOLLIN | EPOLLET | EPOLLRDHUP);
				}
				else
					closeClientConnection(client_fd);
			}
		}
	}
	catch (const std::exception& e) 
	{
		LOG_ERROR("Client " + Utils::toString(client_fd) + " error: " + e.what());
		closeClientConnection(client_fd);
	}
}

void ServerManager::closeClientConnection(int client_fd) 
{
	try 
	{
		_epollManager.removeFd(client_fd);
		close(client_fd);
		delete _clients[client_fd];
		_clients.erase(client_fd);
		LOG_DEBUG("Closed client connection (fd: " + Utils::toString(client_fd) + ")");
	} 
	catch (const std::exception& e) 
	{
		LOG_ERROR("Error closing client (fd: " + Utils::toString(client_fd) + "): " + e.what());
	}
}

void ServerManager::checkClientTimeouts() 
{
	time_t now = time(NULL);
	std::vector<int> timed_out_clients;

	// Identify timed-out clients
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) 
		if (now - it->second->getLastActivity() >= CONNECTION_TIMEOUT) 
			timed_out_clients.push_back(it->first);

	// Close timed-out clients
	for (size_t i = 0; i < timed_out_clients.size(); ++i) 
	{
		LOG_INFO("Closing timed-out client (fd: " + Utils::toString(timed_out_clients[i]) + ")");
		closeClientConnection(timed_out_clients[i]);
	}
}
