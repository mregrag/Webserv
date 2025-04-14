/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 17:36:21 by mregrag           #+#    #+#             */
/*   Updated: 2025/04/14 16:49:48 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Client.hpp"

Client::Client(int fd_client) : _fd(fd_client), _requestComplete(false), _bytesSent(0) {}

Client::~Client() {}

int Client::getFd() const {
	return _fd;
}

void Client::setServer(ServerConfig* server) { _server = server; }
ServerConfig* Client::getServer() const { return _server; }

const std::string& Client::getWriteBuffer() const {
	return _writeBuffer;
}

std::string& Client::getWriteBuffer() {
	return _writeBuffer;
}

const std::string& Client::getReadBuffer() const {
	return _readBuffer;
}

bool Client::isRequestComplete() const {
	return _requestComplete;
}

HTTPRequest& Client::getRequest() {
	return _request;
}

size_t& Client::getBytesSent() {
	return _bytesSent;
}

void Client::appendToBuffer(const char* data, size_t length) {
	_readBuffer.append(data, length);
}

void Client::parseRequest() {
	_request = HTTPRequest(_readBuffer);
	_requestComplete = _request.isComplete();
}

void Client::setResponse(const std::string& response) {
	_writeBuffer = response;
	_bytesSent = 0;
}

void Client::clearBuffers() {
	_readBuffer.clear();
	_writeBuffer.clear();
	_requestComplete = false;
	_bytesSent = 0;
}

