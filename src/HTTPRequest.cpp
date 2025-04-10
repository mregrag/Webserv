#include "../include/HTTPRequest.hpp"
#include <sstream>
#include <cctype>

HTTPRequest::HTTPRequest() : _method(""), _url(""), _version(""), _body("") 
{
}

HTTPRequest::HTTPRequest(const HTTPRequest &other)
	: _method(other._method),
	_url(other._url),
	_version(other._version),
	_headers(other._headers),
	_body(other._body)
{
}

HTTPRequest &HTTPRequest::operator=(const HTTPRequest &rhs)
{
	if (this != &rhs)
	{
		_method = rhs._method;
		_url = rhs._url;
		_version = rhs._version;
		_headers = rhs._headers;
		_body = rhs._body;
	}
	return *this;
}

HTTPRequest::~HTTPRequest() 
{
}

void HTTPRequest::clear()
{
	_method = "";
	_url = "";
	_version = "";
	_headers.clear();
	_body = "";
}

void HTTPRequest::parse()
{
	clear();
	/*// Find the end of the header section.*/
	/*size_t header_end = raw.find("\r\n\r\n");*/
	/*if (header_end == std::string::npos)*/
	/*	return false;*/
	/**/
	/*std::istringstream stream(raw.substr(0, header_end));*/
	/*std::string request_line;*/
	/*if (!std::getline(stream, request_line))*/
	/*	return false;*/
	/**/
	/*// Parse the request line: METHOD URL HTTP/VERSION*/
	/*std::istringstream req_line_stream(request_line);*/
	/*if (!(req_line_stream >> _method >> _url >> _version))*/
	/*	return false;*/
	/**/
	/*// Process the headers.*/
	/*std::string header_line;*/
	/*while (std::getline(stream, header_line))*/
	/*{*/
	/*	// Remove trailing '\r' if present.*/
	/*	if (!header_line.empty() && header_line[header_line.size() - 1] == '\r')*/
	/*		header_line.erase(header_line.size() - 1);*/
	/*	if (header_line.empty())  // Blank line indicates end of headers.*/
	/*		break;*/
	/**/
	/*	size_t colon_pos = header_line.find(":");*/
	/*	if (colon_pos != std::string::npos)*/
	/*	{*/
	/*		std::string key = header_line.substr(0, colon_pos);*/
	/*		std::string value = header_line.substr(colon_pos + 1);*/
	/*		// Trim leading spaces in value.*/
	/*		size_t first_non_space = value.find_first_not_of(" ");*/
	/*		if (first_non_space != std::string::npos)*/
	/*			value = value.substr(first_non_space);*/
	/*		_headers[key] = value;*/
	/*	}*/
	/*}*/
	/**/
	/*// The body starts after the header terminator.*/
	/*_body = raw.substr(header_end + 4);*/
}

const std::string &HTTPRequest::getMethod() const
{
	return _method;
}

const std::string &HTTPRequest::getUrl() const
{
	return _url;
}

const std::string &HTTPRequest::getVersion() const
{
	return _version;
}

const std::map<std::string, std::string> &HTTPRequest::getHeaders() const
{
	return _headers;
}

const std::string &HTTPRequest::getBody() const
{
	return _body;
}

