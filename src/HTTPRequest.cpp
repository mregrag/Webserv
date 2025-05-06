#include "../include/HTTPRequest.hpp"
#include "../include/Utils.hpp"
#include "../include/webserver.hpp"
#include <algorithm>
#include "../include/Logger.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>
#include <cstdlib>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>

HTTPRequest::HTTPRequest() :
	_client(NULL),
	_statusCode(0),
	_state(INIT),
	_parsePosition(0),
	_contentLength(0),
	_method(""),
	_uri(""),
	_path(""),
	_query(""),
	_protocol(""),
	_bodyBuffer(""),
	_tmpHeaderKey(""),
	_tmpHeaderValue(""),
	_headers(),
	_contentType(""),
	_boundary(""),
	_isChunked(false),
	_host(""),
	_keepAlive(true),
	_chunkSize(-1),
	_multipartBoundary(""),
	_matchedLocation(NULL),
	_finalLocation(NULL),
	_uploadWriting(false),
	_uploadHeadersParsed(false)
{
}

HTTPRequest::HTTPRequest(Client* client) :
	_client(client),
	_statusCode(0),
	_state(INIT),
	_parsePosition(0),
	_contentLength(0),
	_method(""),
	_uri(""),
	_path(""),
	_query(""),
	_protocol(""),
	_bodyBuffer(""),
	_tmpHeaderKey(""),
	_tmpHeaderValue(""),
	_headers(),
	_contentType(""),
	_boundary(""),
	_isChunked(false),
	_host(""),
	_keepAlive(true),
	_chunkSize(-1),
	_multipartBoundary(""),
	_matchedLocation(NULL),
	_finalLocation(NULL),
	_uploadWriting(false),
	_uploadHeadersParsed(false)
{
}

HTTPRequest::HTTPRequest(const HTTPRequest& other) :
	_client(other._client),
	_statusCode(other._statusCode),
	_state(other._state),
	_parsePosition(other._parsePosition),
	_contentLength(other._contentLength),
	_method(other._method),
	_uri(other._uri),
	_path(other._path),
	_query(other._query),
	_protocol(other._protocol),
	_bodyBuffer(other._bodyBuffer),
	_tmpHeaderKey(other._tmpHeaderKey),
	_tmpHeaderValue(other._tmpHeaderValue),
	_headers(other._headers),
	_contentType(other._contentType),
	_boundary(other._boundary),
	_isChunked(other._isChunked),
	_host(other._host),
	_keepAlive(other._keepAlive),
	_chunkSize(other._chunkSize),
	_multipartBoundary(other._multipartBoundary),
	_matchedLocation(other._matchedLocation),
	_finalLocation(other._finalLocation),
	
	_uploadWriting(other._uploadWriting),
	_uploadHeadersParsed(other._uploadHeadersParsed)

{
}

HTTPRequest& HTTPRequest::operator=(const HTTPRequest& other) 
{
	if (this != &other) 
	{
		/*closeCurrentFile(); // Close any open stream*/
		_client = other._client;
		_statusCode = other._statusCode;
		_state = other._state;
		_parsePosition = other._parsePosition;
		_contentLength = other._contentLength;
		_method = other._method;
		_uri = other._uri;
		_path = other._path;
		_query = other._query;
		_protocol = other._protocol;
		_bodyBuffer = other._bodyBuffer;
		_tmpHeaderKey = other._tmpHeaderKey;
		_tmpHeaderValue = other._tmpHeaderValue;
		_headers = other._headers;
		_contentType = other._contentType;
		_boundary = other._boundary;
		_isChunked = other._isChunked;
		_host = other._host;
		_keepAlive = other._keepAlive;
		_chunkSize = other._chunkSize;
		_multipartBoundary = other._multipartBoundary;
		_matchedLocation = other._matchedLocation;
		_finalLocation = other._finalLocation;
		_uploadWriting = other._uploadWriting;
		_uploadHeadersParsed = other._uploadHeadersParsed;
	}
	return *this;
}

HTTPRequest::~HTTPRequest() 
{
	/*closeCurrentFile();*/
}

bool HTTPRequest::keepAlive() const
{
	return true;
}

const std::string&	HTTPRequest::getUri() const
{
	return _uri;
}

const std::string&	HTTPRequest::getBody() const
{
	return _bodyBuffer;
}

const std::string& HTTPRequest::getMethod() const 
{
	return _method;
}

const std::string& HTTPRequest::getPath() const 
{
	return _path;
}

int HTTPRequest::getState() const 
{
	return _state;
}

int HTTPRequest::getStatusCode() const 
{
	return _statusCode;
}

void HTTPRequest::setStatusCode(int code) 
{
	_statusCode = code;
	_state = FINISH;
}

void HTTPRequest::setState(ParseState state) 
{
	_state = state;
}

int	HTTPRequest::checkCgi(void)
{
	return (0);
} 

const std::string& HTTPRequest::getHeaderValue(const std::string& key) const 
{
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);
	if (it != _headers.end()) 
		return it->second;
	static std::string empty;
	return empty;
}

bool HTTPRequest::validateClientMaxBodySize() 
{

	if (this->_headers.find("Content-Length") != this->_headers.end())
		this->_contentLength = Utils::stringToSizeT(this->_headers["Content-Length"]);
	if (this->_contentLength > this->_client->getServer()->getClientMaxBodySize())
		return (this->setStatusCode(413), false);
	return true;
}

bool HTTPRequest::validateTransferEncoding() 
{
	if (this->_headers.find("Transfer-Encoding") != this->_headers.end()) 
	{
		std::string value = this->_headers["Transfer-Encoding"];
		if (this->_headers["Transfer-Encoding"] == "chunked")
		{
			this->_isChunked = true;
			return true;
		}
		this->setStatusCode(501); // Not Implemented: Only chunked is supported
		return false;
	}
	return false; // No Transfer-Encoding header
}


bool HTTPRequest::validateAllowedMethods() 
{
	if (_finalLocation->isMethodAllowed(this->_method)) 
		return true;
	this->setStatusCode(405); // Method Not Allowed
	return false;
}


const LocationConfig* HTTPRequest::findLocationByPath(const std::string& path) const 
{
	const std::map<std::string, LocationConfig>& locations = 
		_client->getServer()->getLocations();

	// 1. Check exact match
	std::map<std::string, LocationConfig>::const_iterator it = locations.find(path);
	if (it != locations.end()) {
		return &it->second;
	}

	// 2. Find best prefix match
	std::string bestMatch;
	for (it = locations.begin(); it != locations.end(); ++it) 
	{
		if (path.compare(0, it->first.length(), it->first) == 0 && it->first.length() > bestMatch.length()) 
			bestMatch = it->first;
	}

	if (!bestMatch.empty()) 
		return &locations.find(bestMatch)->second;

	// 3. Fallback to default location
	it = locations.find("/");
	return (it != locations.end()) ? &it->second : NULL;
}

void HTTPRequest::findLocation() 
{
	_matchedLocation = findLocationByPath(_path);
	_finalLocation = _matchedLocation;

	// Follow redirects if any
	if (_matchedLocation && _matchedLocation->is_location_have_redirection()) 
	{
		_finalLocation = findLocationByPath(_matchedLocation->getRedirectPath());
	}
}

const LocationConfig* HTTPRequest::getMatchedLocation() const 
{
	return _matchedLocation;
}

const LocationConfig* HTTPRequest::getFinalLocation() const {
	return _finalLocation;
}

void HTTPRequest::parse(std::string& rawdata)
{
	if (_state == FINISH || rawdata.empty()) 
		return;


	if (_state == INIT) 
		_state = LINE_METHOD;
	if (_state == LINE_METHOD) 
		parseMethod(rawdata);
	if (_state == LINE_URI) 
		parseUri(rawdata);
	if (_state == LINE_VERSION) 
		parseVersion(rawdata);
	if (_state == HEADER_KEY) 
		parseHeadersKey(rawdata);
	if (_state == HEADER_VALUE) 
		parseHeadersValue(rawdata);
	if(_state == BODY_INIT || _state == BODY_MULTIPART)
		parseRequestBody(rawdata);
	
	if (_statusCode == 0 && _state == FINISH) 
	{
		debugPrintRequest();
		_statusCode = 200;
	}
}

void HTTPRequest::parseMethod(std::string& rawdata) 
{
	if (rawdata.empty())
		return;

	size_t space_pos = rawdata.find(' ');
	if (space_pos == std::string::npos)
		return;

	_method = rawdata.substr(0, space_pos);
	if (_method.empty() || !Utils::isValidMethodToken(_method))
		return setStatusCode(400);

	if (!Utils::isSupportedMethod(_method))
		return setStatusCode(501);

	rawdata.erase(0, space_pos + 1);  // Erase only after successful parse
	setState(LINE_URI);
}


void HTTPRequest::parseUri(std::string& rawdata) 
{
	if (rawdata.empty())
		return;

	size_t space_pos = rawdata.find(' ');
	if (space_pos == std::string::npos)
		return; // Wait until we receive the full URI segment

	std::string uri_candidate = rawdata.substr(0, space_pos);

	if (uri_candidate.empty() || !Utils::isValidUri(uri_candidate))
		return setStatusCode(400);

	if (Utils::urlDecode(uri_candidate) == -1)
		return setStatusCode(400);

	_uri = uri_candidate;

	size_t query_pos = _uri.find('?');
	if (query_pos != std::string::npos) 
	{
		_path = _uri.substr(0, query_pos);
		_query = _uri.substr(query_pos + 1);
	} 
	else 
	{
		_path = _uri;
		_query.clear();
	}

	rawdata.erase(0, space_pos + 1);
	findLocation();
	setState(LINE_VERSION);
}

void HTTPRequest::parseVersion(std::string& rawdata) 
{
	if (rawdata.empty())
		return;

	size_t crlf_pos = rawdata.find("\r\n");
	if (crlf_pos == std::string::npos)
		return;

	_protocol = rawdata.substr(0, crlf_pos);

	if (_protocol.empty() || !Utils::isValidVersion(_protocol))
		return setStatusCode(400);

	if (_protocol != "HTTP/1.1")
		return setStatusCode(400);

	rawdata.erase(0, crlf_pos + 2); // Only erase after successful parsing
	setState(HEADER_KEY);
}


void HTTPRequest::parseHeadersKey(std::string& rawdata) 
{
	if (rawdata.empty()) 
		return;

	// Check for header end (\r\n)
	if (rawdata.size() >= 2 && rawdata.substr(0, 2) == "\r\n") 
	{
		if (!_tmpHeaderKey.empty()) 
			return(setStatusCode(400));
		rawdata.erase(0, 2);
		return(setState(BODY_INIT));
	}

	size_t colon_pos = rawdata.find(':');
	if (colon_pos == std::string::npos) 
		return; 

	_tmpHeaderKey = rawdata.substr(0, colon_pos);

	if (!Utils::isValidHeaderKey(_tmpHeaderKey)) 
		return(setStatusCode(400));
	if (_tmpHeaderKey.empty()) 
		return(setStatusCode(400));

	rawdata.erase(0, colon_pos + 1); // Erase key and colon
	setState(HEADER_VALUE);
}

void HTTPRequest::parseHeadersValue(std::string& rawdata) 
{
	if (rawdata.empty()) 
		return;

	// Skip leading whitespace
	size_t start_pos = Utils::skipLeadingWhitespace(rawdata);
	rawdata.erase(0, start_pos);

	size_t crlf_pos = rawdata.find("\r\n");
	if (crlf_pos == std::string::npos) 
		return;

	_tmpHeaderValue = rawdata.substr(0, crlf_pos);

	if (!Utils::isValidHeaderValue(_tmpHeaderValue)) 
		return(setStatusCode(400));
	if (_tmpHeaderValue.empty()) 
		return(setStatusCode(400));

	if (_headers.find(_tmpHeaderKey) != _headers.end()) 
		return(setStatusCode(400));

	_headers[_tmpHeaderKey] = _tmpHeaderValue;
	_tmpHeaderKey.clear();
	_tmpHeaderValue.clear();

	rawdata.erase(0, crlf_pos + 2);
	setState(HEADER_KEY);
	parse(rawdata);
}

void HTTPRequest::parseRequestBody(std::string& rawdata)
{
    if (!validateAllowedMethods() || !validateClientMaxBodySize())
        return;

    if (_state == BODY_INIT)
    {
	    std::string contentType = getHeaderValue("Content-Type");
	    size_t pos = contentType.find("boundary=");
	    if (pos == std::string::npos)
		    return setStatusCode(400);

	    _uploadBoundary = "--" + contentType.substr(pos + 9);
	    _uploadWriting = true;
	    _uploadHeadersParsed = false;
	    _multipartState = PART_HEADER;
	    _state = BODY_MULTIPART;
    }
    if (_state == BODY_MULTIPART)
	    parseMultipartBody(rawdata);
    else
        _state = FINISH;
}


void HTTPRequest::parseMultipartBody(std::string& rawdata) 
{
	while (!rawdata.empty()) 
	{
		switch (_multipartState) 
		{
			case PART_HEADER:
				if (!processPartHeader(rawdata)) 
					return;
				break;

			case PART_DATA:
				if (!processPartData(rawdata)) 
					return;
				break;

			case PART_BOUNDARY:
				if (!processPartBoundary(rawdata)) 
					return;
				break;

			case PART_END:
				_state = FINISH;
				return;
		}
	}
}

bool HTTPRequest::processPartHeader(std::string& rawdata) 
{
	size_t headerEnd = rawdata.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
		return false;

	std::string headers = rawdata.substr(0, headerEnd);
	size_t filenamePos = headers.find("filename=\"");
	if (filenamePos == std::string::npos) 
		return (setStatusCode(400), false);

	// Extract and sanitize filename
	filenamePos += 10;
	size_t endQuote = headers.find("\"", filenamePos);
	if (endQuote == std::string::npos) 
		return (setStatusCode(400), false);

	std::string filename = headers.substr(filenamePos, endQuote - filenamePos);
	for (size_t i = 0; i < filename.size(); ++i) 
	{
		char c = filename[i];
		if (!isalnum(c) && c != '.' && c != '_') 
			filename[i] = '_';
	}

	// Open output file
	_uploadFilepath = _finalLocation->getUploadPath() + "/" + filename;
	_uploadFile.open(_uploadFilepath.c_str(), std::ios::binary);
	if (!_uploadFile.is_open()) 
		return (setStatusCode(500), false);

	rawdata.erase(0, headerEnd + 4); // Remove headers
	_multipartState = PART_DATA;
	return true;
}

bool HTTPRequest::processPartData(std::string& rawdata) 
{
	size_t boundaryPos = rawdata.find(_uploadBoundary);
	if (boundaryPos == std::string::npos) 
	{
		// Write all available data and wait for more
		_uploadFile.write(rawdata.c_str(), rawdata.size());
		rawdata.clear();
		return false;
	}

	// Write data up to boundary
	size_t dataEnd = boundaryPos;
	if (boundaryPos >= 2 && rawdata.substr(boundaryPos - 2, 2) == "\r\n") 
		dataEnd -= 2; // Skip CRLF before boundary

	if (dataEnd > 0) 
		_uploadFile.write(rawdata.c_str(), dataEnd);
	_uploadFile.close();

	// Prepare for boundary processing
	rawdata.erase(0, boundaryPos);
	_multipartState = PART_BOUNDARY;
	return true;
}

bool HTTPRequest::processPartBoundary(std::string& rawdata) 
{
	if (rawdata.substr(0, _uploadBoundary.length()) != _uploadBoundary) 
		return (setStatusCode(400), false);

	rawdata.erase(0, _uploadBoundary.length());

	if (rawdata.substr(0, 2) == "--") 
	{
		// Final boundary
		rawdata.erase(0, 2);
		_multipartState = PART_END;
		return true;
	} 
	else if (rawdata.substr(0, 2) == "\r\n") 
	{
		// Next part
		rawdata.erase(0, 2);
		_uploadFilepath.clear();
		_multipartState = PART_HEADER;
		return true;
	}
	return (setStatusCode(400), false);
}

void HTTPRequest::debugPrintRequest()
{
	std::cout << "------------------------------" << std::endl;
	std::cout << "method  : " << _method << std::endl;
	std::cout << "_uri  : " << _uri << std::endl;
	std::cout << "_path  : " << _path << std::endl;
	std::cout << "_query  : " << _query << std::endl;
	std::cout << "_protocol  : " << _protocol << std::endl;
	std::cout << "==== Headers ====" << std::endl;
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); 
			it != _headers.end(); ++it) {
		std::cout << it->first << ": " << it->second << std::endl;
	}
}


