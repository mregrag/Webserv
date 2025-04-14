#include "../include/webserver.hpp"
#include "../include/ServerManager.hpp"
#include "../include/LocationConfig.hpp"

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
	_clients[client_fd]->setServer(&server);

	char ip_str[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, INET_ADDRSTRLEN);

	LOG_INFO("Accepted new connection from " + std::string(ip_str) + " on server " + server.getServerName() + " (fd: " + toString(client_fd) + ")");
}

void ServerManager::handleClientRequest(int client_fd)
{
	char buffer[4096];
	ssize_t bytes_read;


	Client	*client = _clients[client_fd];
	while ((bytes_read = recv(client_fd, buffer, sizeof (buffer), 0)) > 0)
		client->appendToBuffer(buffer, bytes_read);

	std::cout << buffer << std::endl;
	if (bytes_read == 0)
	{
		LOG_INFO("Client disconnected (fd: " + toString(client_fd) + ")");
		closeConnection(client_fd);
		return ;
	}
	if (bytes_read < 0)
		ErrorServer("receive Error");

	LOG_INFO("Client receive data to (fd: " + toString(client_fd) + ")");
	client->parseRequest();
	if (client->isRequestComplete())
	{
		buildResponse(client);
		LOG_INFO("Client prepare responce (fd: " + toString(client_fd) + ")");
		_epollManager.modifyFd(client_fd, EPOLLOUT | EPOLLET);
	}
}

void ServerManager::buildResponse(Client* client)
{
    int fd = client->getFd();
    LOG_DEBUG("Building response for fd: " + toString(fd));

    // Get the server that accepted this client
    ServerConfig* server_config = client->getServer();
    if (!server_config)
    {
        LOG_ERROR("No server config found for fd: " + toString(fd));
        client->setResponse("HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n");
        return;
    }

    const std::map<std::string, LocationConfig>& locations = server_config->getLocations();
    std::string req_path = client->getRequest().getPath();

    LocationConfig best_match;
    std::string matched_prefix;
    for (std::map<std::string, LocationConfig>::const_iterator loc_it = locations.begin(); loc_it != locations.end(); ++loc_it)
    {
        if (req_path.find(loc_it->first) == 0 && loc_it->first.length() > matched_prefix.length())
        {
            matched_prefix = loc_it->first;
            best_match = loc_it->second;
        }
    }

    std::string full_path = best_match.getRoot();
    if (req_path == "/" && !best_match.getIndex().empty())
        full_path += "/" + best_match.getIndex();
    else
        full_path += req_path;

    std::ifstream file(full_path.c_str(), std::ios::in | std::ios::binary);
    if (!file)
    {
        LOG_ERROR("File not found: " + full_path);
        client->setResponse("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
        return;
    }

    std::stringstream body_stream;
    body_stream << file.rdbuf();
    file.close();
    std::string body = body_stream.str();

    // Determine content type based on file extension
    std::string content_type = "text/plain";
    size_t dot_pos = full_path.find_last_of(".");
    if (dot_pos != std::string::npos)
    {
        std::string extension = full_path.substr(dot_pos);
        if (extension == ".html" || extension == ".htm")
            content_type = "text/html";
        else if (extension == ".css")
            content_type = "text/css";
        else if (extension == ".js")
            content_type = "application/javascript";
        else if (extension == ".json")
            content_type = "application/json";
        else if (extension == ".png")
            content_type = "image/png";
        else if (extension == ".jpg" || extension == ".jpeg")
            content_type = "image/jpeg";
        else if (extension == ".gif")
            content_type = "image/gif";
        else if (extension == ".svg")
            content_type = "image/svg+xml";
        else if (extension == ".ico")
            content_type = "image/x-icon";
        else if (extension == ".txt")
            content_type = "text/plain";
        else if (extension == ".pdf")
            content_type = "application/pdf";
    }

    std::stringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Length: " << body.length() << "\r\n"
             << "Content-Type: " << content_type << "\r\n"
             << "Connection: close\r\n"
             << "\r\n"
             << body;

    client->setResponse(response.str());
    LOG_DEBUG("Built response from file: " + full_path + " with content type: " + content_type);
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
