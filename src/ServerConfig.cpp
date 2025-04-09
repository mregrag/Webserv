/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zel-oirg <zel-oirg@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 17:27:45 by mregrag           #+#    #+#             */
/*   Updated: 2025/04/10 00:11:01 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ServerConfig.hpp"
#include <cstring>
ServerConfig::ServerConfig() : _host(""), _port(0), _serverName(""), _clientMaxBodySize(0)
{
}

ServerConfig::~ServerConfig()
{
}

ServerConfig::ServerConfig(const ServerConfig& other)
{
	*this = other;
}

ServerConfig& ServerConfig::operator=(const ServerConfig& other)
{
	if (this != &other)
	{
		_host = other._host;
		_port = other._port;
		_serverName = other._serverName;
		_clientMaxBodySize = other._clientMaxBodySize;
		_errorPages = other._errorPages;
		_locations = other._locations;
		_listen_fd = other._listen_fd;
		_server_address = other._server_address;
	}
	return *this;
}

bool ServerConfig::isValidHost(const std::string& host)
{
	struct in_addr addr;
	return (inet_pton(AF_INET, host.c_str(), &addr) == 1);
}

void ServerConfig::setHost(const std::string& host)
{
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET; 
	hints.ai_socktype = SOCK_STREAM;

	std::string resolvedHost = host;
	if (getaddrinfo(host.c_str(), NULL, &hints, &res) == 0)
	{
		struct sockaddr_in* ipv4 = reinterpret_cast<struct sockaddr_in*>(res->ai_addr);
		resolvedHost = inet_ntoa(ipv4->sin_addr);
		freeaddrinfo(res);
	}
	else
		throw ErrorServer("Failed to resolve hostname: " + host);

	if (!isValidHost(resolvedHost))
		throw ErrorServer("Wrong syntax: host");

	_host = resolvedHost;
}

void ServerConfig::setPort(const std::string& Port)
{
	if (Port.empty())
		throw ErrorServer("Wrong syntax: port (empty string)");

	for (size_t i = 0; i < Port.length(); ++i)
		if (!std::isdigit(Port[i]))
			throw ErrorServer("Wrong syntax: port (non-digit character)");

	std::stringstream ss(Port);
	int port;
	ss >> port;

	if (ss.fail() || !ss.eof()) 
		throw ErrorServer("Wrong syntax: port (invalid format)");
	if (port < 1 || port > 65535)
		throw ErrorServer("Wrong syntax: port (out of range)");

	_port = static_cast<uint16_t>(port);
}


void ServerConfig::setServerName(const std::string& name)
{
	_serverName = name;
}

void ServerConfig::setClientMaxBodySize(size_t size)
{
	_clientMaxBodySize = size;
}
void ServerConfig::setErrorPage(int code, const std::string& path)
{
	_errorPages[code] = path;
}

void	ServerConfig::setFd(int fd)
{
	this->_listen_fd = fd;
}

void ServerConfig::addLocation(const std::string& path, const LocationConfig& location)
{
	_locations[path] = location;
}

const std::string& ServerConfig::getHost() const
{
	return (_host);
}

int ServerConfig::getPort() const
{
	return (_port);
}

const std::string& ServerConfig::getServerName() const
{
	return (_serverName);
}
size_t ServerConfig::getClientMaxBodySize() const
{
	return (_clientMaxBodySize);
}
const std::map<int, std::string>& ServerConfig::getErrorPages() const
{
	return (_errorPages);
}

int   	ServerConfig::getFd() 
{ 
	return (this->_listen_fd); 
}

const std::map<std::string, LocationConfig>& ServerConfig::getLocations() const
{ 
	return (_locations);
}
void ServerConfig::setupServer()
{
	_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listen_fd == -1)
		throw ErrorServer("Failed to create socket");

	int opt = 1;
	if (setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		throw ErrorServer("Failed to set socket options");

	memset(&_server_address, 0, sizeof(_server_address));
	_server_address.sin_family = AF_INET;
	_server_address.sin_addr.s_addr = inet_addr(_host.c_str());
	_server_address.sin_port = htons(_port);

	if (bind(_listen_fd, (struct sockaddr*)&_server_address, sizeof(_server_address)) == -1)
		throw ErrorServer("Failed to bind socket");

	if (listen(_listen_fd, SOMAXCONN) == -1)
		throw ErrorServer("Failed to listen on socket");
}


int ServerConfig::acceptConnection()
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int client_fd = accept(_listen_fd, (struct sockaddr*)&client_addr, &client_len);
	if (client_fd == -1)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return -1;
		std::cout << "Failed to accept connection" << std::endl;
		return -1;
	}

	std::cout << "New client connected: " << inet_ntoa(client_addr.sin_addr) 
		<< ":" << ntohs(client_addr.sin_port) << std::endl;
	return client_fd;
}

/*void ServerConfig::print() const*/
/*{*/
/*	std::cout << "Server Config:\n";*/
/*	std::cout << "  Host: " << _host << "\n";*/
/*	std::cout << "  Port: " << _port << "\n";*/
/*	std::cout << "  Server Name: " << _serverName << "\n";*/
/*	std::cout << "  Client Max Body Size: " << _clientMaxBodySize << "\n";*/
/*	std::cout << "  Error Pages:\n";*/
/*	for (std::map<int, std::string>::const_iterator it = _errorPages.begin(); it != _errorPages.end(); ++it) {*/
/*		std::cout << "    " << it->first << ": " << it->second << "\n";*/
/*	}*/
/*	for (std::map<std::string, LocationConfig>::const_iterator it = _locations.begin(); it != _locations.end(); ++it) {*/
/*		std::cout << "  Location: " << it->first << "\n";*/
/*		it->second.print();*/
/*	}*/
/*}*/
/**/
/**/
