#include "../include/webserver.hpp"

ServerManager::ServerManager()
{
	LOG_INFO("ServerManager initialized");
}

ServerManager::~ServerManager()
{
	for (std::map<int, Client*>::iterator it = _clients.begin()
		; it != _clients.end(); ++it) 
		closeConnection(it->first);

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
	LOG_INFO("Initializing " + toString(_servers.size()) + " servers");

	for (size_t i = 0; i < _servers.size(); ++i)
	{
		try
		{
			_servers[i].setupServer();
			int listen_fd = _servers[i].getFd();

			if (fcntl(listen_fd, F_SETFL, O_NONBLOCK) == -1)
			{
				LOG_ERROR("Failed to set non-blocking mode for fd: "
						+ toString(listen_fd));
				throw std::runtime_error("fcntl failed");
			}
			_epollManager.addFd(listen_fd, EPOLLIN | EPOLLET);
			_server_map[listen_fd] = &_servers[i];
			LOG_INFO("Server created: " + _servers[i].getServerName() + " on " 
				+ _servers[i].getHost() + ":" + toString(_servers[i].getPort()) 
				+ " (fd: " + toString(listen_fd) + ")");
		}
		catch (const std::exception& e)
		{
			LOG_ERROR("Failed to setup server " + toString(i) + ": " + e.what());
		}
	}
}

void ServerManager::handleEvent(struct epoll_event& event)
{
	int fd = event.data.fd;

	if (_server_map.find(fd) != _server_map.end())
	{
		
		acceptConnection(*_server_map[fd]);
	}
	else if (event.events & EPOLLIN)
	{
		handleClientRequest(fd);
	}
	else if (event.events & EPOLLOUT)
	{
		
		sendResponse(fd);
	}
	else 
	{

		closeConnection(fd);
	}
}

void ServerManager::run()
{
	std::vector<struct epoll_event> events(MAX_EVENTS);

	LOG_INFO("Server started and running");

	while (true)
	{
		int num_events = _epollManager.wait(events, -1);
		LOG_DEBUG("Processing " + toString(num_events) + " events");

		for (int i = 0; i < num_events; ++i)
		{
			try 
			{
				handleEvent(events[i]);
			}
			catch (const std::exception& e) 
			{
				LOG_ERROR("Error handling event: " + std::string(e.what()));
				if (_clients.find(events[i].data.fd) != _clients.end()) 
					closeConnection(events[i].data.fd);
			}
		}
	}
}

void ServerManager::acceptConnection(ServerConfig& server) 
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);
	memset(&client_addr, 0, client_len);

	int client_fd = accept(server.getFd(), (struct sockaddr*)&client_addr, &client_len);
	if (client_fd == -1) 
		throw ErrorServer("Accept failed");

	if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1) 
		throw ErrorServer("Failed to set client socket non-blocking");

	_epollManager.addFd(client_fd, EPOLLIN | EPOLLET | EPOLLRDHUP);
	_clients[client_fd] = new Client(client_fd);

	char ip_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, INET_ADDRSTRLEN);

	LOG_INFO("Accepted new connection from " + std::string(ip_str) + " on server " 
		+ server.getServerName() + " (fd: " + toString(client_fd) + ")");
}

void ServerManager::handleClientRequest(int client_fd)
{
	char buffer[4096];
	ssize_t bytes_read;

	Client	*client = _clients[client_fd];
	while ((bytes_read = recv(client_fd, buffer, sizeof (buffer), 0)) > 0)
		client->appendToBuffer(buffer, bytes_read);
	if (bytes_read == 0 || (bytes_read == -1 && errno != EAGAIN && errno != EWOULDBLOCK))
	{
		LOG_INFO("Client disconnected (fd: " + toString(client_fd) + ")");
		closeConnection(client_fd);
		return ;
	}
	LOG_INFO("Client receive data to (fd: " + toString(client_fd) + ")");
	client->parseRequest();
	if (client->isRequestComplete())
	{
		client->prepareResponse();
		LOG_INFO("Client prepare responce (fd: " + toString(client_fd) + ")");
		_epollManager.modifyFd(client_fd, EPOLLOUT | EPOLLET);
	}
}

void ServerManager::sendResponse(int client_fd) 
{
	Client *client = _clients[client_fd];
	if (!client)
		return ;

	std::string	resp = client->getWriteBuffer();
	ssize_t byteSend = send(client_fd, resp.c_str(), resp.size(), 0);
	if (byteSend == -1)
	{
		LOG_ERROR("Send failed (fd: " + toString(client_fd) + ")");
		closeConnection(client_fd);
		return ;
	}
	LOG_DEBUG("Sent response (fd: " + toString(client_fd) + ")");
	closeConnection(client_fd);
}

void ServerManager::closeConnection(int client_fd) 
{
	_epollManager.removeFd(client_fd);
	close(client_fd);
	delete _clients[client_fd];
	_clients.erase(client_fd);
	LOG_DEBUG("Closed connection (fd: " + toString(client_fd) + ")");
}

