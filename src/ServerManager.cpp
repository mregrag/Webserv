#include "../include/ServerManager.hpp"
#include "../include/HTTPResponse.hpp"
#include "../include/Utils.hpp"
#include "../include/Logger.hpp"
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <errno.h>


ServerManager::ServerManager(const std::vector<ServerConfig>& servers)
	: _epollManager(DEFAULT_MAX_EVENTS),
	_running(false),
	_clientTimeout(DEFAULT_CLIENT_TIMEOUT),
	_maxClients(DEFAULT_MAX_CLIENTS),
	_servers(servers)
{
	LOG_INFO("ServerManager initialized with timeout=" + Utils::toString(_clientTimeout) +
	  "s, maxClients=" + Utils::toString(_maxClients));
}

ServerManager::~ServerManager()
{
	LOG_INFO("ServerManager shutting down");
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) 
	{
		close(it->first);
		delete it->second;
	}
	_clients.clear();
}

bool ServerManager::init()
{
	for (size_t i = 0; i < _servers.size(); ++i) 
	{
		try {
			_servers[i].setupServer();
			const std::vector<int>& fds = _servers[i].getFds();
			const std::vector<uint16_t>& ports = _servers[i].getPorts();
			for (size_t j = 0; j < fds.size(); ++j) 
			{
				_serverMap[fds[j]] = &_servers[i];
				if (!_epollManager.add(fds[j], EPOLLIN)) 
				{
					LOG_ERROR("Failed to add server socket " + Utils::toString(fds[j]) + " to epoll");
					return false;
				}
				LOG_INFO("Server socket " + Utils::toString(fds[j]) + " listening on " + _servers[i].getHost() + ":" + Utils::toString(ports[j]));
			}
		} 
		catch (const std::exception& e) 
		{
			LOG_ERROR("Error setting up server: " + std::string(e.what()));
			return false;
		}
	}
	return true;
}

#include <sys/wait.h>
#include <cstdlib>

void ServerManager::handleEvent(const epoll_event& event) 
{
	int fd = event.data.fd;
	uint32_t events = event.events;

	ServerConfig* server = findServerByFd(fd);
	if (server) 
	{
		if (events & EPOLLIN)
			acceptClient(fd, server);
		return;
	}

	Client* client = findClientByFd(fd);
	if (!client) 
	{
		_epollManager.remove(fd);
		return;
	}

	if (events & (EPOLLERR | EPOLLHUP)) 
	{
		cleanupClient(fd, (events & EPOLLERR) ? "Socket error" : "Connection hangup");
		return;
	}

	if (events & EPOLLIN) 
	{
		if (!receiveFromClient(client))
			cleanupClient(fd, "Connection complete");
	}
	else if (events & EPOLLOUT) 
	{

		if (client->getRequest()->isCgiRequest())
		{

			if (waitpid(client->getRequest()->pid, NULL, WNOHANG) != 0)
			{
				LOG_INFO("cgi finished");

				// setting response
				if (client->getRequest()->first)
				{
					client->getRequest()->first = false;

					std::fstream	file(client->getRequest()->filename);
					std::stringstream	ss;
					ss << file.rdbuf();
					client->getResponse()->appendToBody(ss.str());

					std::remove(client->getRequest()->filename.c_str());
					
					client->getResponse()->setProtocol(client->getRequest()->getProtocol());
					client->getResponse()->setStatusCode(client->getRequest()->getStatusCode());
					client->getResponse()->setStatusMessage(Utils::getMessage(client->getRequest()->getStatusCode()));
					client->getResponse()->setHeader("Connection", client->getResponse()->shouldCloseConnection() ? "close" : "keep-alive");
					client->getResponse()->setHeader("Content-Type", "text/html");
					client->getResponse()->setHeader("Content-Length", Utils::toString(client->getResponse()->getContentLength()));
					client->getResponse()->buildHeader();
					client->getResponse()->setState(HTTPResponse::FINISH);
				}

				// read response form that file and send reponse

			}
			else
			{
				LOG_INFO("cgi still working");

				if (checkClientTimeouts())
				{
					std::remove(client->getRequest()->filename.c_str());
					for (int i = 0; i < client->getRequest()->pids.size(); i++)
						kill(client->getRequest()->pids[i], SIGKILL);
					client->getRequest()->pids.clear();
				}

				// check time out

				// if timeout
					// kill all proccesses
				// else
					// return

				return ;

			}

		}

		if (!sendToClient(client))
			cleanupClient(fd, "Connection complete");
	}
}


void ServerManager::run() 
{
	_running = true;
	LOG_INFO("Starting server event loop");

	while (_running) 
	{
		int numEvents = _epollManager.wait(-1);
		if (numEvents < 0) 
		{
			if (errno == EINTR)
				continue;
			LOG_ERROR("Epoll wait error: " + std::string(std::strerror(errno)));
			break;
		}
		for (int i = 0; i < numEvents; ++i) 
		{
			const epoll_event& event = _epollManager.getEvent(i);
			handleEvent(event);
		}
		checkClientTimeouts();
	}
	LOG_INFO("Server event loop stopped");
}

void ServerManager::stop()
{
	LOG_INFO("Stopping server");
	_running = false;
}

void ServerManager::setClientTimeout(int seconds)
{
	_clientTimeout = seconds;
	LOG_INFO("Client timeout set to " + Utils::toString(seconds) + " seconds");
}

size_t ServerManager::getActiveClientCount() const
{
	return _clients.size();
}

ServerConfig* ServerManager::findServerByFd(int fd)
{
	std::map<int, ServerConfig*>::iterator it = _serverMap.find(fd);
	if (it != _serverMap.end())
		return it->second;
	return NULL;
}

Client* ServerManager::findClientByFd(int fd) 
{
	std::map<int, Client*>::iterator it = _clients.find(fd);
	if (it != _clients.end())
		return it->second;
	return NULL;
}

bool ServerManager::acceptClient(int serverFd, ServerConfig* serverConfig)
{
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	int clientFd = accept(serverFd, (struct sockaddr*)&clientAddr, &clientAddrLen);
	if (clientFd == -1) 
	{
		LOG_ERROR("Failed to accept client connection: " + std::string(std::strerror(errno)));
		return false;
	}

	if (_clients.size() >= static_cast<size_t>(_maxClients)) {

		LOG_WARN("Max clients reached (" + Utils::toString(_maxClients) + "), rejecting new connection");
		close(clientFd);
		return false;
	}

	int flags = fcntl(clientFd, F_GETFL, 0);
	if (flags == -1 || fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) == -1) 
	{
		LOG_ERROR("Failed to set client socket to non-blocking mode: " + std::string(std::strerror(errno)));
		close(clientFd);
		return false;
	}

	try 
	{
		Client* client = new Client(clientFd, serverConfig);
		_clients[clientFd] = client;
		if (!_epollManager.add(clientFd, EPOLLIN)) 
		{
			LOG_ERROR("Failed to add client socket to epoll: " + std::string(std::strerror(errno)));
			delete client;
			_clients.erase(clientFd);
			close(clientFd);
			return false;
		}
		std::string clientIp = inet_ntoa(clientAddr.sin_addr);
		int clientPort = ntohs(clientAddr.sin_port);
		LOG_DEBUG("New client connected from " + clientIp + ":" + Utils::toString(clientPort) + " (fd: " + Utils::toString(clientFd) + ", total: " + Utils::toString(_clients.size()) + ")");
		return true;
	} 
	catch (const std::exception& e) 
	{
		LOG_ERROR("Error creating client: " + std::string(e.what()));
		close(clientFd);
		return false;
	}
}

bool ServerManager::receiveFromClient(Client* client)
{
	if (!client)
		return false;

	char buffer[BUFFER_SIZE];
	std::memset(buffer, 0, BUFFER_SIZE);
	ssize_t bytesRead = recv(client->getFd(), buffer, sizeof(buffer) - 1, 0);
	if (bytesRead > 0) 
	{
		client->setReadBuffer(buffer, bytesRead);
		client->getRequest()->parse(client->getReadBuffer());
		if (client->getRequest()->isComplete()) 
		{
			client->getResponse()->buildResponse();
			if (!_epollManager.modify(client->getFd(), EPOLLOUT)) 
			{
				LOG_ERROR("Failed to modify client socket to write mode (fd: " + Utils::toString(client->getFd()) + ")");
				return false;
			}
		}
		return true;
	}
	if (bytesRead == 0) 
	{
		LOG_INFO("Client fd " + Utils::toString(client->getFd()) + " closed connection");
		return false;
	}
	LOG_ERROR("Error receiving from client fd " + Utils::toString(client->getFd()) + ": " + std::string(std::strerror(errno)));
	return false;
}

bool ServerManager::sendToClient(Client* client)
{

	LOG_INFO("sending to client");
	
	if (!client)
		return false;
	char buffer[BUFFER_SIZE];
	ssize_t bytesToSend = client->getResponseChunk(buffer, sizeof(buffer));
	if (bytesToSend > 0) 
	{
		// std::cout << "a" << std::endl;
		// std::cout << buffer << std::endl;
		// std::cout << "a" << std::endl;

		ssize_t bytesSent = send(client->getFd(), buffer, bytesToSend, 0);
		if (bytesSent <= 0)
		{
			LOG_ERROR("Error sending to client fd " + Utils::toString(client->getFd()) + ": " + std::string(std::strerror(errno)));
			return false;
		}
		return true;
	}
	if (bytesToSend == 0) 
	{
		if (!client->shouldKeepAlive()) 
		{
			LOG_INFO("Response complete, closing connection for client fd " + Utils::toString(client->getFd()));
			return false;
		}
		client->reset();

		LOG_INFO("modifying epoll");
		client->getRequest()->first = true; //?
		if (!_epollManager.modify(client->getFd(), EPOLLIN)) 
		{
			LOG_ERROR("Failed to modify client socket back to read mode (fd: " + Utils::toString(client->getFd()) + ")");
			return false;
		}

		LOG_DEBUG("Keeping connection alive for client fd " + Utils::toString(client->getFd())); //?

		return true;
	}
	LOG_ERROR("Error generating response for client fd " + Utils::toString(client->getFd()));
	return false;
}

void ServerManager::cleanupClient(int fd, const std::string& reason)
{
	std::map<int, Client*>::iterator it = _clients.find(fd);
	if (it != _clients.end()) 
	{
		LOG_INFO("Client disconnected (fd: " + Utils::toString(fd) + ", reason: " + reason + ", remaining: " + Utils::toString(_clients.size() - 1) + ")");
		delete it->second;
		_clients.erase(it);
	}
	_epollManager.remove(fd);
	close(fd);
}

bool ServerManager::checkClientTimeouts()
{
	if (_clientTimeout <= 0)
		return false;
	time_t currentTime = time(NULL);
	std::vector<int> timeoutFds;
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) 
	{
		time_t lastActivity = it->second->getLastActivity();
		if ((currentTime - lastActivity) >= _clientTimeout)
			timeoutFds.push_back(it->first);
	}
	for (size_t i = 0; i < timeoutFds.size(); ++i)
		cleanupClient(timeoutFds[i], "Connection timed out");
	if (!timeoutFds.empty())
	{
		LOG_INFO("Cleaned up " + Utils::toString(timeoutFds.size()) + " client(s) due to " + Utils::toString(_clientTimeout) + "s timeout");
		return (true);
	}
	return (false); //?
}
