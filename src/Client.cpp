/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 17:36:21 by mregrag           #+#    #+#             */
/*   Updated: 2025/04/10 22:08:28 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"

Client::Client() : _client_socket(-1)
{
	memset(&_client_address, 0, sizeof(_client_address));
}

Client::Client(const Client &other)
{
	*this = other;
}

Client::Client(ServerConfig &config) : server(config), _client_socket(-1)
{
	memset(&_client_address, 0, sizeof(_client_address));
}

Client &Client::operator=(const Client &rhs)
{
	if (this != &rhs)
	{
		_client_socket = rhs._client_socket;
		_client_address = rhs._client_address;
		server = rhs.server;
		request = rhs.request;
		response = rhs.response;
	}
	return *this;
}

Client::~Client()
{
}

const int &Client::getSocket() const
{
	return _client_socket;
}

const struct sockaddr_in &Client::getAddress() const
{
	return _client_address;
}

void Client::setSocket(int socket)
{
	_client_socket = socket;
}

void Client::setAddress(const struct sockaddr_in &addr)
{
	_client_address = addr;
}

void Client::setServer(const ServerConfig &config)
{
	server = config;
}

void Client::buildResponse()
{
	response.setStatusCode(200);
	response.setHeader("Content-Type", "text/plain");
	response.setBody("Hello, World!");
}

void Client::clearClient()
{
	request.clear();
	response.clear();
	_client_socket = -1;
	memset(&_client_address, 0, sizeof(_client_address));
}

