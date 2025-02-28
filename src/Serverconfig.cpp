/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Serverconfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 17:27:45 by mregrag           #+#    #+#             */
/*   Updated: 2025/02/25 23:20:25 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ServerConfig.hpp"
#include <iostream>

// ServerConfig Implementation
ServerConfig::ServerConfig() : _port(0), _clientMaxBodySize(0) {}

ServerConfig::ServerConfig(const ServerConfig& other) :
    _port(other._port),
    _host(other._host),
    _serverName(other._serverName),
    _clientMaxBodySize(other._clientMaxBodySize),
    _errorPages(other._errorPages),
    _locations(other._locations) {}

ServerConfig& ServerConfig::operator=(const ServerConfig& other) {
    if (this != &other) {
        _port = other._port;
        _host = other._host;
        _serverName = other._serverName;
        _clientMaxBodySize = other._clientMaxBodySize;
        _errorPages = other._errorPages;
        _locations = other._locations;
    }
    return *this;
}

ServerConfig::~ServerConfig() {}

int ServerConfig::getPort() const {
    return _port;
}

const std::string& ServerConfig::getHost() const {
    return _host;
}

const std::string& ServerConfig::getServerName() const {
    return _serverName;
}

size_t ServerConfig::getClientMaxBodySize() const {
    return _clientMaxBodySize;
}

const std::map<int, std::string>& ServerConfig::getErrorPages() const {
    return _errorPages;
}

const std::vector<LocationConfig>& ServerConfig::getLocations() const {
    return _locations;
}

void ServerConfig::setPort(int port) {
    _port = port;
}

void ServerConfig::setHost(const std::string& host) {
    _host = host;
}

void ServerConfig::setServerName(const std::string& serverName) {
    _serverName = serverName;
}

void ServerConfig::setClientMaxBodySize(size_t size) {
    _clientMaxBodySize = size;
}

void ServerConfig::addErrorPage(int code, const std::string& path) {
    _errorPages[code] = path;
}

void ServerConfig::addLocation(const LocationConfig& location) {
    _locations.push_back(location);
}
