#include "../include/Client.hpp"

Client::Client(int fd_client) : _fd(fd_client), _requestComplete(false) {}

Client::~Client() {}

int	Client::getFd() const 
{
	return _fd;
}

const std::string& Client::getWriteBuffer() const
{
	return _writeBuffer;
}

bool Client::isRequestComplete() const
{
	return _requestComplete;
}

void Client::appendToBuffer(const char* data, size_t length)
{
	_readBuffer.append(data, length);
}

void Client::parseRequest()
{
	_request = HTTPRequest(_readBuffer);
	_requestComplete = _request.isComplete();
}

void Client::prepareResponse()
{
	std::string body = "<html><body><h1>Hello from " + _request.getPath() + "</h1></body></html>";
	std::stringstream response;
	response << "HTTP/1.1 200 OK\r\n"
			 << "Content-Length: " << body.length() << "\r\n"
			 << "Content-Type: text/html\r\n"
			 << "\r\n"
			 << body;
	_writeBuffer = response.str();
}

void Client::clearWriteBuffer()
{
	_writeBuffer.clear();
}