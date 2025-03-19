/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Serverconfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 17:27:45 by mregrag           #+#    #+#             */
/*   Updated: 2025/03/19 01:54:35 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ServerConfig.hpp"

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
	}
	return *this;
}

bool isValidHost(const std::string& host)
{
	struct in_addr addr;
	if (inet_pton(AF_INET, host.c_str(), &addr) == 1)
		return (true);

	return (false);
}

void ServerConfig::setHost(const std::string& host)
{
	std::string Host = host;

	if (Host == "localhost")
		Host = "127.0.0.1";
	if (!isValidHost(Host))
		throw std::runtime_error("Wrong syntax: host");
	_host = Host;
}


void ServerConfig::setPort(const std::string& Port)
{
	if (Port.empty())
		throw std::runtime_error("Wrong syntax: port (empty string)");

	for (size_t i = 0; i < Port.length(); ++i)
		if (!std::isdigit(Port[i]))
			throw std::runtime_error("Wrong syntax: port (non-digit character)");

	std::stringstream ss(Port);
	int port;
	ss >> port;

	if (ss.fail() || !ss.eof()) 
		throw std::runtime_error("Wrong syntax: port (invalid format)");
	if (port < 1 || port > 65535)
		throw std::runtime_error("Wrong syntax: port (out of range)");

	this->_port = static_cast<uint16_t>(port);
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
void ServerConfig::addLocation(const std::string& path, const LocationConfig& location)
{
	_locations[path] = location;
}

const std::string& ServerConfig::getHost() const
{
	return _host;
}

int ServerConfig::getPort() const
{
	return _port;
}
const std::string& ServerConfig::getServerName() const
{
	return _serverName;
}
size_t ServerConfig::getClientMaxBodySize() const
{
	return _clientMaxBodySize;
}
const std::map<int, std::string>& ServerConfig::getErrorPages() const
{
	return _errorPages;
}
const std::map<std::string, LocationConfig>& ServerConfig::getLocations() const
{ 
	return _locations;
}

// Socket and server setup
void ServerConfig::setupServer()
{
	createSocket() ;
	bindSocket();
	listenForConnections();
}

void ServerConfig::startServer()
{
	std::cout << "Server is running on " << _host << ":" << _port << "\n";
	acceptConnections();
}

void ServerConfig::createSocket()
{
	_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listen_fd == -1)
		throw std::runtime_error("Failed to create socket");

	// Set socket options to reuse address
	int opt = 1;
	if (setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		throw std::runtime_error("Failed to set socket options");
}

void ServerConfig::bindSocket()
{
	_server_address.sin_family = AF_INET;
	_server_address.sin_addr.s_addr = inet_addr(_host.c_str());
	_server_address.sin_port = htons(_port);

	if (bind(_listen_fd, (struct sockaddr*)&_server_address, sizeof(_server_address)) == -1)
		throw std::runtime_error("Failed to bind socket");
}

void ServerConfig::listenForConnections()
{
	if (listen(_listen_fd, 10) == -1)
		throw std::runtime_error("Failed to listen on socket");
}

void ServerConfig::acceptConnections()
{
	while (true)
	{
		struct sockaddr_in client_address;
		socklen_t client_len = sizeof(client_address);
		int client_fd = accept(_listen_fd, (struct sockaddr*)&client_address, &client_len);
		if (client_fd == -1) {
			std::cout << "Failed to accept connection\n";
			continue;
		}

		char client_ip[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
		std::cout << "Accepted connection from " << client_ip << ":" << ntohs(client_address.sin_port) << "\n";

		close(client_fd);
	}
}

void ServerConfig::print() const
{
	std::cout << "Server Config:\n";
	std::cout << "  Host: " << _host << "\n";
	std::cout << "  Port: " << _port << "\n";
	std::cout << "  Server Name: " << _serverName << "\n";
	std::cout << "  Client Max Body Size: " << _clientMaxBodySize << "\n";
	std::cout << "  Error Pages:\n";
	for (std::map<int, std::string>::const_iterator it = _errorPages.begin(); it != _errorPages.end(); ++it) {
		std::cout << "    " << it->first << ": " << it->second << "\n";
	}
	for (std::map<std::string, LocationConfig>::const_iterator it = _locations.begin(); it != _locations.end(); ++it) {
		std::cout << "  Location: " << it->first << "\n";
		it->second.print();
	}
}
