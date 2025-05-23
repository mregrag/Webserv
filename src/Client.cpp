#include "../include/Client.hpp"
#include "../include/Logger.hpp"
#include "../include/HTTPRequest.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>


Client::Client(int fd, ServerConfig* server)
	: _fd(fd)
	, _server(server)
	, _request(this->getServer())              
	, _response(this->getRequest())            
	, _bytesSent(0)
	, _headersSize(0)
	, _lastActivity(time(NULL))
	, _readBuffer("")
{
	if (fd < 0 || !server)
		throw std::runtime_error("Invalid client parameters");
}

Client::~Client()
{
	if (_fileStream.is_open())
		_fileStream.close();

	if (_fd >= 0)
		close(_fd);

	LOG_DEBUG("Client destroyed, fd: " + Utils::toString(_fd));
}

std::string& Client::getReadBuffer()
{
	return _readBuffer;
}

void Client::setReadBuffer(const char* data, ssize_t bytesSet)
{
	_readBuffer.append(data, bytesSet);
}

void Client::setClientAddress(const struct sockaddr_in& addr)
{
	_addr = addr;
}

ssize_t Client::getResponseChunk(char* buffer, size_t bufferSize)
{
	if (!buffer)
		return -1;

	updateActivity();

	const size_t totalResponseSize = _response.getHeader().size() + _response.getContentLength();
	if (_bytesSent >= totalResponseSize)
		return 0;

	if (_bytesSent < _response.getHeader().size())
		return getHeaderResponse(buffer, bufferSize);
	else if (!_response.getBody().empty())
		return getStringBodyResponse(buffer, bufferSize);
	else if (!_response.getFilePath().empty())
		return getFileBodyResponse(buffer, bufferSize);

	return 0;
}

ssize_t Client::getHeaderResponse(char* buffer, size_t bufferSize)
{
	size_t headerRemaining = _response.getHeader().size() - _bytesSent;
	size_t bytesToCopy = std::min(headerRemaining, bufferSize);

	memcpy(buffer, _response.getHeader().c_str() + _bytesSent, bytesToCopy);
	_bytesSent += bytesToCopy;

	if (_bytesSent >= _response.getHeader().size())
		_headersSize = _response.getHeader().size();

	return bytesToCopy;
}

ssize_t Client::getStringBodyResponse(char* buffer, size_t bufferSize)
{
	const std::string& body = _response.getBody();
	size_t bodyOffset = _bytesSent - _headersSize;

	if (bodyOffset >= body.size())
		return 0;

	size_t bytesToCopy = std::min(body.size() - bodyOffset, bufferSize);
	memcpy(buffer, body.c_str() + bodyOffset, bytesToCopy);
	_bytesSent += bytesToCopy;

	return bytesToCopy;
}

ssize_t Client::getFileBodyResponse(char* buffer, size_t bufferSize)
{
	if (!buffer || bufferSize == 0)
		return -1;

	const std::string& filePath = _response.getFilePath();

	// Open file if not already opened
	if (!_fileStream.is_open())
	{
		_fileStream.open(filePath.c_str(), std::ios::binary);
		if (!_fileStream.is_open())
			return (LOG_ERROR("Failed to open file: " + filePath), -1);
	}

	_fileStream.read(buffer, bufferSize);
	ssize_t bytesRead = _fileStream.gcount();

	if (bytesRead > 0)
	{
		_bytesSent += bytesRead;
		return bytesRead;
	}

	_fileStream.close();
	return 0;
}

int Client::getFd() const
{
	return _fd;
}

time_t Client::getLastActivity() const
{
	return _lastActivity;
}

void Client::updateActivity()
{
	_lastActivity = time(NULL);
}

ClientState Client::getState() const
{
	return _state;
}

HTTPRequest* Client::getRequest()
{
	return &_request;
}

HTTPResponse* Client::getResponse()
{
	return &_response;
}

ServerConfig* Client::getServer() const
{
	return _server;
}

bool Client::shouldKeepAlive() const
{
	return !_response.shouldCloseConnection();
}

bool Client::hasTimedOut(time_t currentTime, time_t timeout) const
{
	return (currentTime - _lastActivity) > timeout;
}

void Client::reset()
{
	_request.clear();
	_response.clear();
	_readBuffer.clear();
	_bytesSent = 0;
	_headersSize = 0;

	if (_fileStream.is_open())
		_fileStream.close();

	updateActivity();
	// LOG_DEBUG("Client reset for keep-alive, fd: " + Utils::toString(_fd)); //?
}
