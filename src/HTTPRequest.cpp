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

HTTPRequest::HTTPRequest()
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
	_HeaderKey(""),
	_HeaderValue(""),
	_headers(),
	_contentType(""),
	_boundary(""),
	_isChunked(false),
	_keepAlive(true),
	_chunkSize(-1),
	_multipartBoundary(""),
	_matchedLocation(NULL),
	_location(NULL),
	_uploadHeadersParsed(false),
	_chunkFileInitialized(false)
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
	_HeaderKey(other._HeaderKey),
	_HeaderValue(other._HeaderValue),
	_headers(other._headers),
	_contentType(other._contentType),
	_boundary(other._boundary),
	_isChunked(other._isChunked),
	_keepAlive(other._keepAlive),
	_chunkSize(other._chunkSize),
	_multipartBoundary(other._multipartBoundary),
	_matchedLocation(other._matchedLocation),
	_location(other._location),
	_uploadHeadersParsed(other._uploadHeadersParsed),
	_chunkFileInitialized(other._chunkFileInitialized)

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
		_HeaderKey = other._HeaderKey;
		_HeaderValue = other._HeaderValue;
		_headers = other._headers;
		_contentType = other._contentType;
		_boundary = other._boundary;
		_isChunked = other._isChunked;
		_keepAlive = other._keepAlive;
		_chunkSize = other._chunkSize;
		_multipartBoundary = other._multipartBoundary;
		_matchedLocation = other._matchedLocation;
		_location = other._location;
		_uploadHeadersParsed = other._uploadHeadersParsed;
		_chunkFileInitialized = other._chunkFileInitialized;
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
const std::string&	HTTPRequest::getQuery() const
{
	return _query;
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



const std::string& HTTPRequest::getHeaderValue(const std::string& key) const 
{
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);
	if (it != _headers.end()) 
		return it->second;
	static std::string empty;
	return empty;
}

const LocationConfig* HTTPRequest::findLocationByPath(const std::string& path) const 
{
	const std::map<std::string, LocationConfig>& locations = _client->getServer()->getLocations();

	// 1. Check exact match
	std::map<std::string, LocationConfig>::const_iterator it = locations.find(path);
	if (it != locations.end()) 
		return &it->second;

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
	_location = _matchedLocation;

	// Follow redirects if any
	if (_matchedLocation && _matchedLocation->hasRedirection()) 
	{
		_location = findLocationByPath(_matchedLocation->getRedirectPath());
	}
}

const LocationConfig* HTTPRequest::getMatchedLocation() const 
{
	return _matchedLocation;
}

const LocationConfig* HTTPRequest::getFinalLocation() const {
	return _location;
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

	rawdata.erase(0, space_pos + 1);
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
		if (!_HeaderKey.empty()) 
			return(setStatusCode(400));
		rawdata.erase(0, 2);
		return(setState(BODY_INIT));
	}

	size_t colon_pos = rawdata.find(':');
	if (colon_pos == std::string::npos) 
		return; 

	_HeaderKey = rawdata.substr(0, colon_pos);

	if (!Utils::isValidHeaderKey(_HeaderKey)) 
		return(setStatusCode(400));
	if (_HeaderKey.empty()) 
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

	_HeaderValue = rawdata.substr(0, crlf_pos);

	if (!Utils::isValidHeaderValue(_HeaderValue))
		return setStatusCode(400);
	if (_HeaderValue.empty())
		return setStatusCode(400);

	if (_headers.find(_HeaderKey) != _headers.end()) 
	{
		// Headers that MUST NOT be duplicated
		if (_HeaderKey == "Host" || _HeaderKey == "Content-Length" || _HeaderKey == "Transfer-Encoding" || _HeaderKey == "Connection" || _HeaderKey == "Expect") 
			return setStatusCode(400);
		// Headers that should be concatenated with commas
		else if (_HeaderKey == "Cookie" || _HeaderKey == "Accept" || _HeaderKey == "Accept-Language") 
			_headers[_HeaderKey] += "; " + _HeaderValue;
		// For other headers, last value wins (override)
		else 
			_headers[_HeaderKey] = _HeaderValue;
	}
	else 
		_headers[_HeaderKey] = _HeaderValue;

	_HeaderKey.clear();
	_HeaderValue.clear();
	rawdata.erase(0, crlf_pos + 2);
	setState(HEADER_KEY);
	parse(rawdata);
}


bool HTTPRequest::validateTransferEncoding()
{
	if (!isCgiRequest())
		return (false);

	std::string transfer_encoding = getHeaderValue("Transfer-Encoding");
	if (!transfer_encoding.empty())
	{
		if (Utils::trim(transfer_encoding) == "chunked")
		{
			this->_isChunked = true;
			_state = BODY_CHUNKED;
			_chunkState = CHUNK_SIZE;
			return true;
		}
		this->setStatusCode(501); // Not Implemented
		return false;
	}

	// Fallback to Content-Length
	std::string content_length = getHeaderValue("Content-Length");
	if (!content_length.empty())
	{
		this->_contentLength = Utils::stringToSizeT(content_length);
		return true;
	}

	this->setStatusCode(411); // Length Required
	return false;
}

bool HTTPRequest::validateMultipartFormData()
{
	if (_state == BODY_CHUNKED)
		return (true);

	std::string content_type = getHeaderValue("Content-Type");
	if (content_type.empty())
		return false;

	if (content_type.find("multipart/form-data") == std::string::npos)
		return false;

	size_t boundary_pos = content_type.find("boundary=");
	if (boundary_pos == std::string::npos)
	{
		this->setStatusCode(400); // Bad Request: no boundary
		return false;
	}

	_uploadBoundary = "--" + content_type.substr(boundary_pos + 9); // 9 = length of "boundary="
	_multipartState = PART_HEADER;
	_state = BODY_MULTIPART;

	return true;
}

bool HTTPRequest::validateClientMaxBodySize() 
{
	if (_isChunked)
		return true; // Skip this check for chunked transfers

	if (this->_headers.find("Content-Length") != this->_headers.end())
		this->_contentLength = Utils::stringToSizeT(this->_headers["Content-Length"]);

	if (this->_contentLength > this->_client->getServer()->getClientMaxBodySize())
		return (this->setStatusCode(413), false); // Payload Too Large

	return true;
}


bool HTTPRequest::validateAllowedMethods() 
{
	if (_location->isMethodAllowed(this->_method)) 
		return true;
	this->setStatusCode(405); 
	return false;
}


void HTTPRequest::parseChunkBody(std::string& rawdata)
{
	static std::ofstream outfile;

	// Initialize file on first chunk
	if (!_chunkFileInitialized)
	{
		std::string temp_path = _client->getServer()->getClientBodyTmpPath();
		std::ostringstream filename;
		filename << temp_path << "/body_"; // Unique name per client
		_chunkFilePath = filename.str();
		outfile.open(_chunkFilePath.c_str(), std::ios::binary | std::ios::out | std::ios::trunc);
		if (!outfile.is_open())
			return(setStatusCode(500));
		_chunkFileInitialized = true;
	}

	while (!rawdata.empty())
	{
		if (_chunkState == CHUNK_SIZE)
		{
			size_t pos = rawdata.find("\r\n");
			if (pos == std::string::npos)
				return; // wait for more data

			std::string size_str = rawdata.substr(0, pos);
			std::istringstream iss(size_str);
			iss >> std::hex >> _chunkSize;
			if (iss.fail())
			{
				setStatusCode(400); // Bad Request
				outfile.close();
				return;
			}
			rawdata.erase(0, pos + 2); // Remove size line

			if (_chunkSize == 0)
			{
				_chunkState = CHUNK_FINISHED;
				_state = FINISH;
				outfile.close(); // Finalize the file
				return;
			}
			_chunkState = CHUNK_DATA;
		}
		else if (_chunkState == CHUNK_DATA)
		{
			if (rawdata.size() < _chunkSize + 2)
				return; // wait for more data

			std::string chunk_data = rawdata.substr(0, _chunkSize);

			// ✅ Write to file instead of buffer
			outfile.write(chunk_data.c_str(), chunk_data.size());

			rawdata.erase(0, _chunkSize + 2); // Remove data + \r\n
			_chunkState = CHUNK_SIZE;
		}
		else if (_chunkState == CHUNK_FINISHED)
		{
			_state = FINISH;
			return;
		}
	}
}

void HTTPRequest::parseRequestBody(std::string& rawdata) 
{

	if (_state == BODY_INIT)
	{
		if (!validateAllowedMethods()) 
			return; 

		if (!isCgiRequest())
			return;

		if (!validateTransferEncoding()) 
			return; 

		if (!validateClientMaxBodySize()) 
			return; 

		if (!validateMultipartFormData())
			return;

	}

	else if (_state == BODY_MULTIPART) 
		parseMultipartBody(rawdata); 
	else if (_state == BODY_CHUNKED) 
		parseChunkBody(rawdata); 
	else 
		_state = FINISH; 
}

void HTTPRequest::parseMultipartBody(std::string& rawdata)
{
	while (!rawdata.empty())
	{
		if (_multipartState == PART_HEADER)
			if (!processPartHeader(rawdata))
				return;
		if (_multipartState == PART_DATA)
			if (!processPartData(rawdata))
				return;
		if (_multipartState == PART_BOUNDARY)
			if (!processPartBoundary(rawdata))
				return;
		if (_multipartState == PART_END)
			return(setState(FINISH));
	}
}

bool HTTPRequest::isCgiRequest()
{
	_hasCgi = false;
	if (_location->hasCgi())
	{
		std::cout << "hello " << std::endl;
		_hasCgi = true;
		return (true);
	}
	return (true);
}

bool HTTPRequest::hasCgi()
{
	return _hasCgi;
}

bool HTTPRequest::processPartHeader(std::string& rawdata)
{
	size_t header_end = rawdata.find("\r\n\r\n");
	if (header_end == std::string::npos)
		return false;

	std::string headers = rawdata.substr(0, header_end);
	rawdata.erase(0, header_end + 4); // Remove headers

	// Extract filename
	size_t filename_pos = headers.find("filename=\"");
	if (filename_pos == std::string::npos)
		return setStatusCode(400), false;

	filename_pos += 10;
	size_t end_quote = headers.find("\"", filename_pos);
	if (end_quote == std::string::npos)
		return setStatusCode(400), false;

	std::string filename = headers.substr(filename_pos, end_quote - filename_pos);
	for (size_t i = 0; i < filename.size(); ++i)
	{
		char c = filename[i];
		if (!isalnum(c) && c != '.' && c != '_')
			filename[i] = '_';
	}

	_uploadFilepath = _location->getUploadPath() + "/" + filename;
	_uploadFile.open(_uploadFilepath.c_str(), std::ios::binary);
	if (!_uploadFile.is_open())
		return setStatusCode(500), false;

	_multipartState = PART_DATA;
	return true;
}

bool HTTPRequest::processPartData(std::string& rawdata)
{
	size_t boundary_pos = rawdata.find(_uploadBoundary);
	if (boundary_pos == std::string::npos)
	{
		size_t tail_reserve = _uploadBoundary.size() + 4; // For CRLF or "--"
		if (rawdata.size() <= tail_reserve)
			return false; // Wait for more data

		size_t write_len = rawdata.size() - tail_reserve;
		_uploadFile.write(rawdata.c_str(), write_len);
		rawdata.erase(0, write_len);
		return false;
	}

	// Write up to boundary, excluding trailing CRLF if present
	size_t data_end = boundary_pos;
	if (boundary_pos >= 2 && rawdata.substr(boundary_pos - 2, 2) == "\r\n")
		data_end -= 2;

	if (data_end > 0)
		_uploadFile.write(rawdata.c_str(), data_end);

	_uploadFile.close();
	rawdata.erase(0, boundary_pos);
	_multipartState = PART_BOUNDARY;
	return true;
}

bool HTTPRequest::processPartBoundary(std::string& rawdata)
{
	if (rawdata.substr(0, _uploadBoundary.length()) != _uploadBoundary)
		return setStatusCode(400), false;

	rawdata.erase(0, _uploadBoundary.length());

	if (rawdata.substr(0, 2) == "--")
	{
		rawdata.erase(0, 2);
		_multipartState = PART_END;
		return true;
	}
	else if (rawdata.substr(0, 2) == "\r\n")
	{
		rawdata.erase(0, 2);
		_uploadFilepath.clear();
		_multipartState = PART_HEADER;
		return true;
	}

	return setStatusCode(400), false;
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


