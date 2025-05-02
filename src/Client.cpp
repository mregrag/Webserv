/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 17:36:21 by mregrag           #+#    #+#             */
/*   Updated: 2025/05/03 00:15:44 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"
#include "../include/Logger.hpp"
#include "../include/HTTPRequest.hpp"
#include "../include/HTTPResponse.hpp"
#include "../include/Utils.hpp"
#include "../include/webserver.hpp"
#include <stdexcept>


Client::Client(int fd, ServerConfig* server) : _fd(fd), _server(server), _bytesSent(0),_request(new HTTPRequest(this)), _response(new HTTPResponse(this)) 
{
	_lastActivity = time(NULL);
}

Client::~Client() 
{
	if (_fd >= 0) 
		close(_fd);
	delete _request;
	delete _response;
}

void Client::handleRead() 
{
	char buffer[BUFFER_SIZE + 1];
	std::memset(buffer, 0, BUFFER_SIZE + 1);

	while (true) 
	{
		int bytesRead = recv(_fd, buffer, BUFFER_SIZE, 0);

		if (bytesRead > 0) 
		{
			buffer[bytesRead] = '\0';
			_readBuffer.append(buffer, bytesRead);
			_request->parse(_readBuffer);
		}
		else if (bytesRead == 0) 
			throw std::runtime_error("Client closed connection");
		else if (bytesRead < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break; // All data read
			throw std::runtime_error("Error reading from socket: " + std::string(strerror(errno)));
		}
	}
}


void Client::handleWrite() 
{
	if (this->_response->buildResponse() == -1) // Reponse not ready
		return ;
	if (this->getResponse()->getState() == HTTPResponse::FINISH)
	{
		if (this->_request->getState() != HTTPRequest::FINISH)
			throw std::runtime_error("Client Close connection");
		LOG_DEBUG("Response sent to client "  + Utils::toString( this->getFd()));
	}


	int bytesSent = -1;
	if (this->getFd() != -1)
		bytesSent = send(this->getFd(), this->_response->getResponse().c_str(), this->_response->getResponse().size(), 0);

	if (bytesSent < 0)
		throw std::runtime_error("Error with send function");
	else
		LOG_DEBUG("Sent " + Utils::toString(bytesSent) + " bytes to client " + Utils::toString(this->getFd()));

	if (this->getResponse()->getState() == HTTPResponse::FINISH)
	{
		if (this->_request->getState() != HTTPRequest::FINISH)
			throw std::runtime_error("Client Close connection");
		LOG_DEBUG("Response sent to client "  + Utils::toString( this->getFd()));
	}

}

int Client::getFd() const {
	return _fd;
}

time_t Client::getLastActivity() const {
	return _lastActivity;
}

ServerConfig* Client::getServer() const {
	return _server;
}

HTTPRequest* Client::getRequest() {
	return _request;
}

HTTPResponse* Client::getResponse() {
	return _response;
}

void Client::updateActivity() {
	_lastActivity = time(NULL);
}

void Client::reset() {
	delete _request;
	delete _response;
	_request = new HTTPRequest(this);
	_response = new HTTPResponse(this);
	_readBuffer.clear();
	_writeBuffer.clear();
	_bytesSent = 0;
	updateActivity();
}

void Client::clearBuffers() {
	_readBuffer.clear();
	_writeBuffer.clear();
	_bytesSent = 0;
}
