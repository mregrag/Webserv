#include "../include/HTTPResponse.hpp"
#include <sstream>

HTTPResponse::HTTPResponse() : _statusCode(200), _statusMessage("OK"), _body("") 
{
}

HTTPResponse::HTTPResponse(const HTTPResponse &other)
	: _statusCode(other._statusCode),
	_statusMessage(other._statusMessage),
	_headers(other._headers),
	_body(other._body)
{
}

HTTPResponse &HTTPResponse::operator=(const HTTPResponse &rhs)
{
	if (this != &rhs)
	{
		_statusCode = rhs._statusCode;
		_statusMessage = rhs._statusMessage;
		_headers = rhs._headers;
		_body = rhs._body;
	}
	return *this;
}

HTTPResponse::~HTTPResponse()
{
}

void HTTPResponse::clear()
{
	_statusCode = 200;
	_statusMessage = "OK";
	_headers.clear();
	_body = "";
}

void HTTPResponse::setStatusCode(int code)
{
	_statusCode = code;
}

void HTTPResponse::setStatusMessage(const std::string &message)
{
	_statusMessage = message;
}

void HTTPResponse::setHeader(const std::string &key, const std::string &value)
{
	_headers[key] = value;
}

void HTTPResponse::setBody(const std::string &body)
{
	_body = body;
	// Update the Content-Length header automatically.
	std::ostringstream oss;
	oss << _body.length();
	_headers["Content-Length"] = oss.str();
}

std::string HTTPResponse::getResponse() const
{
	std::ostringstream response;
	response << "HTTP/1.1 " << _statusCode << " " << _statusMessage << "\r\n";

	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
		response << it->first << ": " << it->second << "\r\n";

	response << "\r\n";  // End of headers.
	response << _body;   // Append the body.
	return response.str();
}

