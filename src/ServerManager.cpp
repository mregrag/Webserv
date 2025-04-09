#include "../include/webserver.hpp"

ServerManager::ServerManager()
{
	LOG_INFO("ServerManager initialized");
}

ServerManager::~ServerManager()
{
	for (std::map<int, std::string>::iterator it = _client_requests.begin(); it != _client_requests.end(); ++it) 
	{
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
	LOG_INFO("Initializing " + toString(_servers.size()) + " servers");

	for (size_t i = 0; i < _servers.size(); ++i)
	{
		try
		{
			_servers[i].setupServer();
			int listen_fd = _servers[i].getFd();

			if (fcntl(listen_fd, F_SETFL, O_NONBLOCK) == -1)
			{
				LOG_ERROR("Failed to set non-blocking mode for fd: " + toString(listen_fd));
				throw std::runtime_error("fcntl failed");
			}

			_epollManager.addFd(listen_fd, EPOLLIN | EPOLLET);
			_server_map[listen_fd] = &_servers[i];

			LOG_INFO("Server created: " + _servers[i].getServerName() + " on " + _servers[i].getHost() + ":" + toString(_servers[i].getPort()) + " (fd: " + toString(listen_fd) + ")");
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
		acceptConnection(*_server_map[fd]);
	else if (event.events & EPOLLIN)
		handleClientRequest(fd);
	else if (event.events & EPOLLOUT)
		sendResponse(fd, "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!");
	else 
		closeConnection(fd);
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
				if (_client_requests.find(events[i].data.fd) != _client_requests.end()) 
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
	_client_requests[client_fd] = "";

	char ip_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, INET_ADDRSTRLEN);

	LOG_INFO("Accepted new connection from " + std::string(ip_str) + " on server " + server.getServerName() + " (fd: " + toString(client_fd) + ")");
}

void ServerManager::handleClientRequest(int client_fd)
{
	char buffer[4096];
	ssize_t bytes_read;

	while ((bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) 
	{
		buffer[bytes_read] = '\0';
		_client_requests[client_fd] += buffer;

		size_t header_end = _client_requests[client_fd].find("\r\n\r\n");
		if (header_end != std::string::npos) 
		{
			LOG_DEBUG("Received complete headers from client (fd: " + toString(client_fd) + ")");

			size_t content_length = 0;
			size_t cl_pos = _client_requests[client_fd].find("Content-Length: ");
			if (cl_pos != std::string::npos) 
				content_length = atoi(_client_requests[client_fd].c_str() + cl_pos + 16);

			if (_client_requests[client_fd].length() >= header_end + 4 + content_length) 
			{
				std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
				_client_requests[client_fd] = response;
				_epollManager.modifyFd(client_fd, EPOLLOUT | EPOLLET);
				break;
			}
		}
	}

	if (bytes_read == 0 || (bytes_read == -1 && errno != EAGAIN && errno != EWOULDBLOCK)) 
	{
		LOG_INFO("Client disconnected (fd: " + toString(client_fd) + ")");
		closeConnection(client_fd);
	}
}

void ServerManager::sendResponse(int client_fd, const std::string& response) 
{
	ssize_t bytes_sent = send(client_fd, response.c_str(), response.length(), 0);

	if (bytes_sent == -1) 
	{
		if (errno != EAGAIN && errno != EWOULDBLOCK) 
		{
			LOG_ERROR("Send failed to client (fd: " + toString(client_fd) + ")");
			closeConnection(client_fd);
		}
		return;
	}

	LOG_DEBUG("Sent response to client (fd: " + toString(client_fd) + ")");
	closeConnection(client_fd);
}

void ServerManager::closeConnection(int client_fd) 
{
	_epollManager.removeFd(client_fd);
	close(client_fd);
	_client_requests.erase(client_fd);
	LOG_DEBUG("Closed connection (fd: " + toString(client_fd) + ")");
}

