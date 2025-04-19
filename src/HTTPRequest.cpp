#include "../include/HTTPRequest.hpp"
#include <algorithm>
#include "../include/Logger.hpp"
#include <sstream>
#include <iostream>
#include <cctype>
#include <cstdlib>

HTTPRequest::HTTPRequest(Client *client) : _client(client), _state(HTTPRequest::INIT), _parsePosition(0), _contentLength(0)
{
}

HTTPRequest::~HTTPRequest() 
{
}

const std::string& HTTPRequest::getMethod() const
{
	return _method;
}

const std::string&  HTTPRequest::getPath() const
{
	return _path;
}

const std::string&  HTTPRequest::getHeaderValue(const std::string& key) const
{
	std::map<std::string, std::string>::const_iterator  it = _headers.find(key);
	if (it != _headers.end())
		return (*it).second;
	throw(std::out_of_range("Header " + key + " is out of range"));
}

void HTTPRequest::setState(e_parse_state state)
{
	if (this->_state == FINISH)
		return;
	if (this->_state == state)
		return;
	this->_state = state;
}
int	HTTPRequest::getState(void)
{
	return (this->_state);
}

void HTTPRequest:: parse(const std::string& rawdata)
{
	if (this->_state == HTTPRequest::FINISH)
		return;
	if (this->_state == HTTPRequest::INIT)
		LOG_DEBUG("psrsing request " + rawdata.substr(0, rawdata.find("\n")));
	this->_request += rawdata;
	if (_request.empty())
	{
		LOG_DEBUG("Empty request");
		return ;
	}


	parseRequestLine();
	parseRequestHeader();
	parseRequestBody();
	std::cout << "------------------------------" << std::endl;
	std::cout << "method  : " << _method << std::endl;
	std::cout << "_uri  : " << _uri << std::endl;
	std::cout << "_path  : " << _path << std::endl;
	std::cout << "_query  : " << _query << std::endl;
	std::cout << "_protocol  : " << _protocol << std::endl;
	std::cout << "==== Headers ====" << std::endl;
	for (std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); ++it)
		std::cout << it->first << ": " << it->second << std::endl;
	std::cout << "----------------_body----------------" << std::endl;
	std::cout << _body << std::endl;
	std::cout << "----------------" << std::endl;
}
void	HTTPRequest::parseRequestLine(void)
{
	if (this->_state > LINE_END)
		return (LOG_DEBUG("Request line already parsed"));
	if (this->_state == HTTPRequest::INIT)
		this->setState(LINE_METHOD);

	if (this->_state == LINE_METHOD)
		this->parseMethod();
	if (this->_state == LINE_PATH)
		this->parseUri();
	if (this->_state == LINE_VERSION)
		this->parseVersion();
	if(this->_state == LINE_END)
		this->checkLineEnd();
}

void HTTPRequest::parseMethod(void)
{
	size_t i = 0;
	size_t raw_size = _request.size();

	while (i < raw_size && _request[i] != ' ')
	{
		if (!std::isalpha(_request[i]))
			return(setStatusCode(400));
		_method += _request[i];
		++i;
	}
	if (i == raw_size)
	{
		_request.erase(0, i);
		return;
	}

	_request.erase(0, i + 1);
	if (_method.empty())
		return(setStatusCode(400));

	if (_method != "GET" && _method != "POST" && _method != "DELETE")
		return(setStatusCode(400));

	LOG_DEBUG("Method: " + _method);

	setState(LINE_PATH);
}

int HTTPRequest::urlDecode(std::string& str)
{
	std::string result;
	size_t i = 0;

	while (i < str.length())
	{
		if (str[i] == '%')
		{
			if (i + 2 >= str.length() || !isxdigit(str[i + 1]) || !isxdigit(str[i + 2]))
				return -1;

			std::string hex = str.substr(i + 1, 2);
			char decodedChar = static_cast<char>(std::strtol(hex.c_str(), NULL, 16));
			result += decodedChar;
			i += 3;
		}
		else if (str[i] == '+')
		{
			result += ' ';
			i++;
		}
		else
		{
			result += str[i];
			i++;
		}
	}

	str = result;
	return 0;
}


void HTTPRequest::parseUri(void)
{
	size_t i = 0;
	size_t raw_size = _request.size();

	while (i < raw_size && (_request[i] == ' ' || _request[i] == '\t'))
		++i;

	// Extract URI until the next space
	while (i < raw_size && _request[i] != ' ')
	{
		if (!std::isprint(_request[i]))
			return(setStatusCode(400));
		_uri += _request[i];
		++i;
	}

	// Erase parsed part + space (if any)
	if (i < raw_size && _request[i] == ' ')
		++i;
	_request.erase(0, i);

	if (_uri.empty())
		return(setStatusCode(400));

	if (urlDecode(_uri) == -1)
		return(setStatusCode(400));

	size_t pos = _uri.find('?');
	if (pos != std::string::npos)
	{
		_path = _uri.substr(0, pos);
		_query = _uri.substr(pos + 1);
	}
	else
		_path = _uri;
	LOG_DEBUG("URI: " + _uri);
	setState(LINE_VERSION);
}


void HTTPRequest::parseVersion(void)
{
	size_t i = 0;
	size_t raw_size = _request.size();

	// Skip initial whitespace
	while (i < raw_size && (_request[i] == ' ' || _request[i] == '\t'))
		++i;

	// Parse version token (e.g., "HTTP/1.1")
	while (i < raw_size)
	{
		char c = _request[i];

		if (c != 'H' && c != 'T' && c != 'P' && c != '/' && c != '.' && !std::isdigit(c))
			break;
		_protocol += c;
		++i;
	}
	_request.erase(0, i);

	if (_protocol.empty())
		return(setStatusCode(400));

	if (_protocol != "HTTP/1.1")
		return(setStatusCode(505));

	setState(LINE_END);
}

void HTTPRequest::checkLineEnd(void)
{
	if (_state != LINE_END)
		return;

	// Trim leading spaces and tabs
	_request.erase(0, _request.find_first_not_of(" \t"));

	if (_request.empty())
		return; 

	if (_request[0] == '\n')
	{
		_request.erase(0, 1);
		return(setState(HEADERS_INIT));
	}
	if (_request[0] == '\r')
	{
		if (_request.size() < 2)
			return; // Wait for \n

		if (_request[1] == '\n')
		{
			_request.erase(0, 2);
			return(setState(HEADERS_INIT));
		}
		return(setStatusCode(400));
	}
	setStatusCode(400);
}




void	HTTPRequest::parseRequestHeader(void)
{
	if (this->_state < HEADERS_INIT)
		return (LOG_DEBUG("Request line not parsed yet"));
	if (this->_state > HEADERS_END)
		return (LOG_DEBUG("Headers already parsed"));

	if (this->_state == HEADERS_INIT)
		this->setState(HEADERS_KEY);

	if (this->_state == HEADERS_KEY)
		this->parseHeaderKey();
	if (this->_state == HEADERS_VALUE)
		this->parseHeaderValue();
	if (_state == HEADERS_END)
		return checkHeaderEnd();
}

void HTTPRequest::parseHeaderKey(void)
{
	// Detect end of headers (empty line or \r\n)
	if (!this->_request.empty() && (this->_request[0] == '\r' || this->_request[0] == '\n'))
	{
		if (this->_request[0] == '\n')
		{
			this->_request.erase(0, 1);
			if (!this->_tmpHeaderKey.empty())
				return this->setStatusCode(400); // Unexpected header key
			return this->setState(BODY_INIT); // End of headers, start body
		}

		// Check for \r\n sequence indicating the end of headers
		if (this->_request.size() < 2)
			return;

		if (this->_request[1] == '\n')
		{
			this->_request.erase(0, 2);
			if (!this->_tmpHeaderKey.empty())
				return this->setStatusCode(400); // Unexpected header key
			return this->setState(BODY_INIT); // End of headers, start body
		}

		return this->setStatusCode(400); // Invalid header format
	}

	// Process the header key
	size_t i = 0;
	size_t rawSize = this->_request.size();
	bool foundColon = false;

	// Parse the header key until we encounter a colon ':'
	while (i < rawSize)
	{
		// Reject spaces and tabs in header keys
		if (this->_request[i] == ' ' || this->_request[i] == '\t')
			return this->setStatusCode(400); // Invalid character in header key

		// If a colon is found, we know we've reached the end of the header key
		if (this->_request[i] == ':')
		{
			foundColon = true;
			break;
		}

		// Ensure that the characters in the header key are valid (alphanumeric, hyphen, underscore)
		if (!std::isalnum(this->_request[i]) && this->_request[i] != '-' && this->_request[i] != '_')
			return this->setStatusCode(400); // Invalid character in header key

		// Add valid characters to the temporary header key
		this->_tmpHeaderKey += this->_request[i];
		i++;
	}

	// Remove the processed part of the request
	this->_request.erase(0, foundColon ? i + 1 : i);

	if (foundColon)
	{
		// If the header key is empty after parsing, it's an error
		if (this->_tmpHeaderKey.empty())
			return this->setStatusCode(400);

		this->_tmpHeaderKey.erase(std::remove_if(this->_tmpHeaderKey.begin(), this->_tmpHeaderKey.end(), ::isspace), this->_tmpHeaderKey.end());

		LOG_DEBUG("Header key: %s" + this->_tmpHeaderKey);
		this->setState(HEADERS_VALUE); 
	}
}

void HTTPRequest::parseHeaderValue(void)
{
	size_t i = 0;
	size_t rawSize = this->_request.size();
	bool foundNonPrintable = false;

	while (i < rawSize)
	{
		if (this->_tmpHeaderValue.empty() && (this->_request[i] == ' ' || this->_request[i] == '\t'))
		{
			i++;
			continue;
		}
		// If a non-printable character is encountered, break the loop
		if (!std::isprint(this->_request[i]))
		{
			foundNonPrintable = true;
			break;
		}
		this->_tmpHeaderValue += this->_request[i];
		i++;
	}
	this->_request.erase(0, i);

	// If a non-printable character was found, handle the error
	if (foundNonPrintable)
	{
		if (this->_tmpHeaderValue.empty())
			return this->setStatusCode(400); 

		// Check if the header already exists in the map (duplicate header)
		if (this->_headers.find(this->_tmpHeaderKey) != this->_headers.end())
			return this->setStatusCode(400); // Duplicate header found

		LOG_DEBUG("Header value: %s" + this->_tmpHeaderValue);
		this->_headers[this->_tmpHeaderKey] = this->_tmpHeaderValue;
		this->_tmpHeaderKey.clear();
		this->_tmpHeaderValue.clear();

		this->setState(HEADERS_END);
	}
}

void HTTPRequest::checkHeaderEnd()
{
	// Remove leading whitespace
	_request.erase(0, _request.find_first_not_of(" \t"));

	if (_request.empty())
		return;
	if (_request[0] == '\n')
	{
		_request.erase(0, 1);
		setState(HEADERS_KEY);
		parseRequestHeader();
		return;
	}
	if (_request[0] == '\r')
	{
		if (_request.size() < 2)
			return; // Wait for \n
		if (_request[1] == '\n')
		{
			_request.erase(0, 2);
			setState(HEADERS_KEY);
			parseRequestHeader();
			return;
		}
		return(setStatusCode(400));
	}
	setStatusCode(400);
}

void HTTPRequest::parseRequestBody() 
{
	if (_state != BODY_INIT) 
		return;

	// Check for Transfer-Encoding (e.g., chunked)
	if (_headers.find("Transfer-Encoding") != _headers.end()) 
	{
		std::string transferEncoding = _headers["Transfer-Encoding"];
		if (transferEncoding.find("chunked") != std::string::npos) 
		{
			LOG_DEBUG("Chunked transfer encoding is not supported");
			return setStatusCode(501); // Not Implemented
		}
		else 
		{
			LOG_DEBUG("Unsupported Transfer-Encoding: " + transferEncoding);
			return setStatusCode(400);
		}
	}
	if (_method != "POST" && _method != "PUT" && _method != "PATCH") 
	{
		_state = FINISH;
		return;
	}

	// Require Content-Length for these methods
	if (_headers.find("Content-Length") == _headers.end()) 
	{
		LOG_DEBUG("Missing Content-Length header");
		return setStatusCode(400);
	}

	// Parse Content-Length once
	if (_contentLength == 0) 
	{
		std::string clStr = _headers["Content-Length"];
		char* end;
		long parsedCl = std::strtol(clStr.c_str(), &end, 10);
		if (*end != '\0' || parsedCl < 0) {
			LOG_DEBUG("Invalid Content-Length: " + clStr);
			return setStatusCode(400); // Bad Request
		}
		_contentLength = static_cast<size_t>(parsedCl);
		LOG_DEBUG("Content-Length: " + clStr);
	}

	// Calculate remaining bytes to read
	size_t remaining = _contentLength - _body.size();
	size_t toRead = std::min(remaining, _request.size());

	// Append data to body and remove from request buffer
	_body.append(_request.substr(0, toRead));
	_request.erase(0, toRead);

	// Check if body is fully received
	if (_body.size() == _contentLength) 
	{
		// Ensure no extra data remains
		if (!_request.empty()) 
			return setStatusCode(400);
		_state = FINISH;
	} 
}

void	HTTPRequest::setStatusCode(int code)
{
	this->_statusCode = code;
	this->setState(HTTPRequest::FINISH);
}
