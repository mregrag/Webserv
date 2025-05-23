#include "../include/HTTPRequest.hpp"
#include "../include/ServerConfig.hpp"
#include "../include/LocationConfig.hpp"
#include "../include/Utils.hpp"
#include <cctype>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>

HTTPRequest::HTTPRequest(ServerConfig* server)
	: _server(server),
	_statusCode(0),
	_state(INIT),
	_parsePosition(0),
	_contentLength(0),
	_method(""),
	_uri("/"),
	_path(""),
	_query(""),
	_protocol("HTTP/1.1"),
	_bodyBuffer(""),
	_headerKey(""),
	_headerValue(""),
	_headers(),
	_contentType(""),
	_boundary(""),
	_resource(""),
	_isChunked(false),
	_keepAlive(true),
	_chunkSize(-1),
	_multipartBoundary(""),
	_location(NULL),
	_uploadHeadersParsed(false),
	_chunkFileInitialized(false),
	_uploadFilepath(""),
	_uploadBoundary(""),
	_multipartState(PART_HEADER),
	_chunkState(CHUNK_SIZE),
	_hasCgi(false),
	_chunkFilePath(""),
	first(true)
{
}

HTTPRequest::HTTPRequest(const HTTPRequest& other)
	: _server(other._server),
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
	_headerKey(other._headerKey),
	_headerValue(other._headerValue),
	_headers(other._headers),
	_contentType(other._contentType),
	_boundary(other._boundary),
	_resource(other._resource),
	_isChunked(other._isChunked),
	_keepAlive(other._keepAlive),
	_chunkSize(other._chunkSize),
	_multipartBoundary(other._multipartBoundary),
	_location(other._location),
	_uploadHeadersParsed(other._uploadHeadersParsed),
	_chunkFileInitialized(other._chunkFileInitialized),
	_uploadFilepath(other._uploadFilepath),
	_uploadBoundary(other._uploadBoundary),
	_multipartState(other._multipartState),
	_chunkState(other._chunkState),
	_hasCgi(other._hasCgi),
	_chunkFilePath(other._chunkFilePath)
{
}

HTTPRequest& HTTPRequest::operator=(const HTTPRequest& other) 
{
	if (this != &other) 
	{
		closeOpenFiles();
		_server               = other._server;
		_statusCode           = other._statusCode;
		_state                = other._state;
		_parsePosition        = other._parsePosition;
		_contentLength        = other._contentLength;
		_method               = other._method;
		_uri                  = other._uri;
		_path                 = other._path;
		_query                = other._query;
		_protocol             = other._protocol;
		_bodyBuffer           = other._bodyBuffer;
		_headerKey            = other._headerKey;
		_headerValue          = other._headerValue;
		_headers              = other._headers;
		_contentType          = other._contentType;
		_boundary             = other._boundary;
		_resource             = other._resource;
		_isChunked            = other._isChunked;
		_keepAlive            = other._keepAlive;
		_chunkSize            = other._chunkSize;
		_multipartBoundary    = other._multipartBoundary;
		_location             = other._location;
		_uploadHeadersParsed  = other._uploadHeadersParsed;
		_chunkFileInitialized = other._chunkFileInitialized;
		_uploadFilepath       = other._uploadFilepath;
		_uploadBoundary       = other._uploadBoundary;
		_multipartState       = other._multipartState;
		_chunkState           = other._chunkState;
		_hasCgi               = other._hasCgi;
		_chunkFilePath        = other._chunkFilePath;
	}
	return *this;
}

HTTPRequest::~HTTPRequest() 
{
	closeOpenFiles();
}

void HTTPRequest::closeOpenFiles() 
{
	if (_uploadFile.is_open())
		_uploadFile.close();
	if (_chunkFile.is_open())
		_chunkFile.close();
}

bool HTTPRequest::keepAlive() const 
{
	return _keepAlive;
}

std::string HTTPRequest::getHeader(const std::string& name) const 
{
	std::string canonical = name;
	bool capitalize = true;
	for (size_t i = 0; i < canonical.size(); ++i) 
	{
		if (capitalize) 
		{
			canonical[i] = std::toupper(canonical[i]);
			capitalize = false;
		}
		else if (canonical[i] == '-') 
			capitalize = true;
		else 
			canonical[i] = std::tolower(canonical[i]);
	}
	std::map<std::string,std::string>::const_iterator it = _headers.find(canonical);
	return (it != _headers.end()) ? it->second : "";
}

const std::map<std::string, std::string>& HTTPRequest::getHeaders() const 
{
	return _headers;
}

bool HTTPRequest::isComplete() const 
{
	return (_state == FINISH || _state == ERRORE);
}


bool HTTPRequest::isCgiRequest()
{
	if (_location && _location->hasCgi())
		return (true);
	return (false);
}


const std::string& HTTPRequest::getUri() const 
{
	return _uri;
}

const std::string& HTTPRequest::getProtocol() const 
{
	return _protocol;
}

const std::string& HTTPRequest::getQuery() const 
{
	return _query;
}

const std::string& HTTPRequest::getBody() const 
{
	return _bodyBuffer;
}

const std::string& HTTPRequest::getMethod() const 
{
	return _method;
}

const std::string& HTTPRequest::getResource() const 
{
	return _resource;
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

ServerConfig* HTTPRequest::getServer() const 
{
    return _server;
}

size_t HTTPRequest::getContentLength() const 
{
	return _contentLength;
}

const std::string& HTTPRequest::getHeaderValue(const std::string& key) const 
{
	std::map<std::string, std::string>::const_iterator it = _headers.find(key);
	static std::string empty;
	return (it != _headers.end()) ? it->second : empty;
}

void HTTPRequest::setStatusCode(int code) 
{
	_statusCode = code;
}

void HTTPRequest::setState(ParseState state) 
{
	_state = state;
}

const LocationConfig* HTTPRequest::getLocation() const 
{
	return _location;
}

void HTTPRequest::parse(std::string& rawdata) 
{
	if (_state == FINISH || _state == ERRORE || rawdata.empty())
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
	if (_state == BODY_INIT || _state == BODY_MULTIPART)
		parseRequestBody(rawdata);
}

void HTTPRequest::parseMethod(std::string& rawdata) 
{
	size_t space_pos = rawdata.find(' ');
	if (space_pos == std::string::npos)
		return;
	_method = rawdata.substr(0, space_pos);
	if (_method.empty() || !Utils::isValidMethodToken(_method)) 
	{
		setState(ERRORE);
		setStatusCode(400);
		return;
	}
	if (!Utils::isSupportedMethod(_method)) 
	{
		setState(ERRORE);
		setStatusCode(400);
		return;
	}
	rawdata.erase(0, space_pos + 1);
	setState(LINE_URI);
}

void HTTPRequest::parseUri(std::string& rawdata) 
{
	size_t space_pos = rawdata.find(' ');
	if (space_pos == std::string::npos)
		return;
	_uri = rawdata.substr(0, space_pos);
	if (_uri.size() > 8192) 
	{
		setState(ERRORE);
		setStatusCode(414);
		return;
	}
	if (_uri.empty() || !Utils::isValidUri(_uri)) 
	{
		setState(ERRORE);
		setStatusCode(400);
		return;
	}
	if (Utils::urlDecode(_uri) == -1) 
	{
		setState(ERRORE);
		setStatusCode(400);
		return;
	}
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

	_location = _server->findLocation(_path);
	_resource = _location->getResource(_path);
	setState(LINE_VERSION);
}

void HTTPRequest::parseVersion(std::string& rawdata) 
{
	size_t pos = rawdata.find("\r\n");
	if (pos == std::string::npos)
		pos = rawdata.find("\n");
	if (pos == std::string::npos)
		return;
	_protocol = rawdata.substr(0, pos);
	if (!Utils::isValidVersion(_protocol) || _protocol != "HTTP/1.1") 
	{
		setState(ERRORE);
		setStatusCode(400);
		return;
	}
	rawdata.erase(0, pos + ((rawdata[pos]=='\r') ? 2 : 1));
	setState(HEADER_KEY);
}

void HTTPRequest::parseHeadersKey(std::string& rawdata) 
{
	if (rawdata.empty())
		return;
	bool isCRLF = (rawdata.size() >= 2 && rawdata.substr(0,2) == "\r\n");
	bool isLF = (!isCRLF && rawdata.size() >= 1 && rawdata[0] == '\n');
	if (isCRLF || isLF) 
	{
		if (_headers.empty() || _headers.find("host") == _headers.end()) 
		{
			setState(ERRORE);
			setStatusCode(400);
			return;
		}
		rawdata.erase(0, (isCRLF ? 2 : 1));
		setState(BODY_INIT);
		return;
	}
	size_t colon_pos = rawdata.find(':');
	if (colon_pos == std::string::npos)
		return;
	_headerKey = rawdata.substr(0, colon_pos);
	if (!Utils::isValidHeaderKey(_headerKey)) 
	{
		setState(ERRORE);
		setStatusCode(400);
		return;
	}
	rawdata.erase(0, colon_pos + 1);
	setState(HEADER_VALUE);
}

void HTTPRequest::parseHeadersValue(std::string& rawdata) 
{
	size_t start = Utils::skipLeadingWhitespace(rawdata);
	rawdata.erase(0, start);
	size_t pos = rawdata.find("\r\n");
	if (pos == std::string::npos)
		pos = rawdata.find("\n");
	if (pos == std::string::npos)
		return;
	_headerValue = rawdata.substr(0, pos);
	if (!Utils::isValidHeaderValue(_headerValue)) 
	{
		setState(ERRORE);
		setStatusCode(400);
		return;
	}
	for (size_t i = 0; i < _headerKey.size(); ++i)
		_headerKey[i] = std::tolower(_headerKey[i]);
	if (_headers.find(_headerKey) != _headers.end()) 
	{
		if (_headerKey == "host" || _headerKey == "content-length" || _headerKey == "transfer-encoding" || _headerKey == "connection" || _headerKey == "expect") 
		{
			setState(ERRORE);
			setStatusCode(400);
			return;
		}
		else if (_headerKey == "cookie" || _headerKey == "accept" || _headerKey == "accept-language") 
			_headers[_headerKey] += "; " + _headerValue;
		else
			_headers[_headerKey] = _headerValue;
	} 
	else 
		_headers[_headerKey] = _headerValue;
	_headerKey.erase();
	_headerValue.erase();
	rawdata.erase(0, pos + ((rawdata[pos]=='\r') ? 2 : 1));
	setState(HEADER_KEY);
	parse(rawdata);
}

void HTTPRequest::parseRequestBody(std::string& rawdata) 
{
	if (_state == BODY_INIT) 
	{
		if (!validateAllowedMethods() ||
			!validateHostHeader() ||
			!validateContentLength() ||
			!validateTransferEncoding() ||
			!validateMultipartFormData())
			return;
	}
	if (_state == BODY_MULTIPART)
		parseMultipartBody(rawdata);
	else if (_state == BODY_CHUNKED)
		parseChunkBody(rawdata);
	else 
	{
		setStatusCode(200);
		setState(FINISH);
	}
}

void HTTPRequest::parseChunkBody(std::string& rawdata) 
{
	if (!_chunkFileInitialized) 
	{
		std::stringstream ss;
		ss << _server->getClientBodyTmpPath() << "/mregrag_" << time(NULL);
		_chunkFilePath = ss.str();
		_chunkFile.open(_chunkFilePath.c_str(), std::ios::binary | std::ios::out);
		if (!_chunkFile.is_open()) {
			setStatusCode(500);
			setState(ERRORE);
			return;
		}
		_chunkFileInitialized = true;
		_chunkState = CHUNK_SIZE;
	}
	while (!rawdata.empty()) 
	{
		if (_chunkState == CHUNK_SIZE) 
		{
			if (!processChunkSize(rawdata))
				return;
		}
		else if (_chunkState == CHUNK_DATA) 
		{
			if (!processChunkData(rawdata))
				return;
		}
		else if (_chunkState == CHUNK_FINISHED)
			return;
	}
}

bool HTTPRequest::processChunkSize(std::string& rawdata) 
{
	size_t pos = rawdata.find("\r\n");
	if (pos == std::string::npos)
		return false;
	if (pos == 0 || pos > 8) 
	{
		setStatusCode(400);
		setState(ERRORE);
		_chunkFile.close();
		return false;
	}
	_chunkSize = 0;
	for (size_t i = 0; i < pos; ++i) 
	{
		char c = rawdata[i];
		if (c >= '0' && c <= '9')
			_chunkSize = (_chunkSize * 16) + (c - '0');
		else if (c >= 'a' && c <= 'f')
			_chunkSize = (_chunkSize * 16) + (c - 'a' + 10);
		else if (c >= 'A' && c <= 'F')
			_chunkSize = (_chunkSize * 16) + (c - 'A' + 10);
		else 
		{
			setStatusCode(400);
			setState(ERRORE);
			_chunkFile.close();
			return false;
		}
	}
	rawdata.erase(0, pos + 2);
	if (_chunkSize == 0) 
	{
		_chunkFile.close();
		_chunkState = CHUNK_FINISHED;
		setStatusCode(201);
		setState(FINISH);
		return false;
	}
	_chunkState = CHUNK_DATA;
	return true;
}

bool HTTPRequest::processChunkData(std::string& rawdata) 
{
	if (rawdata.size() < _chunkSize + 2)
		return false;
	if (rawdata[_chunkSize] != '\r' || rawdata[_chunkSize + 1] != '\n') 
	{
		setStatusCode(400);
		setState(ERRORE);
		_chunkFile.close();
		return false;
	}
	_chunkFile.write(rawdata.c_str(), _chunkSize);
	if (!_chunkFile.good()) 
	{
		setStatusCode(500);
		setState(ERRORE);
		_chunkFile.close();
		return false;
	}
	rawdata.erase(0, _chunkSize + 2);
	_chunkState = CHUNK_SIZE;
	return true;
}

void HTTPRequest::parseMultipartBody(std::string& rawdata) 
{
	while (!rawdata.empty()) 
	{
		if (_multipartState == PART_HEADER) 
		{
			if (!processPartHeader(rawdata))
				return;
		}
		else if (_multipartState == PART_DATA) 
		{
			if (!processPartData(rawdata))
				return;
		}
		else if (_multipartState == PART_BOUNDARY) 
		{
			if (!processPartBoundary(rawdata))
				return;
		} 
		else if (_multipartState == PART_END) 
		{
			setStatusCode(201);
			return;
		}
	}
}

bool HTTPRequest::processPartHeader(std::string& rawdata) 
{
	size_t end = rawdata.find("\r\n\r\n");
	if (end == std::string::npos) 
	{
		setStatusCode(400);
		setState(ERRORE);
		return false;
	}
	std::string headers = rawdata.substr(0, end);
	rawdata.erase(0, end + 4);
	size_t filename_pos = headers.find("filename=\"");
	if (filename_pos == std::string::npos) 
	{
		setStatusCode(400);
		setState(ERRORE);
		return false;
	}
	filename_pos += 10;
	size_t quote_end = headers.find("\"", filename_pos);
	if (quote_end == std::string::npos) 
	{
		setStatusCode(400);
		setState(ERRORE);
		return false;
	}
	std::string filename = headers.substr(filename_pos, quote_end - filename_pos);
	// Sanitize filename.
	for (size_t i = 0; i < filename.size(); ++i)
		if (!std::isalnum(filename[i]) && filename[i] != '.' && filename[i] != '_')
			filename[i] = '_';
	_uploadFilepath = _location->getUploadPath() + "/" + filename;
	_uploadFile.open(_uploadFilepath.c_str(), std::ios::binary);
	if (!_uploadFile.is_open()) 
	{
		setStatusCode(500);
		setState(ERRORE);
		return false;
	}
	_multipartState = PART_DATA;
	return true;
}

bool HTTPRequest::processPartData(std::string& rawdata) 
{
	size_t boundary_pos = rawdata.find(_uploadBoundary);
	if (boundary_pos == std::string::npos) 
	{
		size_t reserve = _uploadBoundary.size() + 4;
		if (rawdata.size() <= reserve)
			return false;
		size_t write_len = rawdata.size() - reserve;
		_uploadFile.write(rawdata.c_str(), write_len);
		_uploadFile.flush();
		rawdata.erase(0, write_len);
		return false;
	}
	size_t data_end = boundary_pos;
	if (boundary_pos >= 2 && rawdata.substr(boundary_pos - 2, 2) == "\r\n")
		data_end -= 2;
	if (data_end > 0) 
	{
		_uploadFile.write(rawdata.c_str(), data_end);
		_uploadFile.flush();
	}
	_uploadFile.close();
	rawdata.erase(0, boundary_pos);
	_multipartState = PART_BOUNDARY;
	return true;
}

bool HTTPRequest::processPartBoundary(std::string& rawdata) 
{
	if (rawdata.substr(0, _uploadBoundary.length()) != _uploadBoundary) 
	{
		setStatusCode(400);
		setState(ERRORE);
		return false;
	}
	rawdata.erase(0, _uploadBoundary.length());
	if (rawdata.substr(0, 2) == "--") 
	{
		rawdata.erase(0, 2);
		_multipartState = PART_END;
		setState(FINISH);
		setStatusCode(201);
		return true;
	}
	else if (rawdata.substr(0, 2) == "\r\n") 
	{
		rawdata.erase(0, 2);
		_uploadFilepath.erase();
		_multipartState = PART_HEADER;
		return true;
	}
	setStatusCode(400);
	setState(ERRORE);
	return false;
}

bool HTTPRequest::validateHostHeader() 
{
	if (_protocol == "HTTP/1.1" && getHeaderValue("host").empty()) 
	{
		setStatusCode(400);
		setState(ERRORE);
		return false;
	}
	return true;
}

bool HTTPRequest::validateContentLength() 
{
	std::string cl = getHeaderValue("content-length");
	if (cl.empty())
		return true;
	for (size_t i = 0; i < cl.size(); ++i)
		if (!std::isdigit(cl[i])) 
		{
			setStatusCode(400);
			setState(ERRORE);
			return false;
		}
	_contentLength = Utils::stringToSizeT(cl);
	if (_contentLength > _server->getClientMaxBodySize()) 
	{
		setStatusCode(413);
		setState(ERRORE);
		return false;
	}
	return true;
}

bool HTTPRequest::validateTransferEncoding() 
{
	std::string te = getHeaderValue("transfer-encoding");
	std::string cl = getHeaderValue("content-length");
	if (!te.empty()) 
	{
		if (te != "chunked") 
		{
			setStatusCode(501);
			setState(ERRORE);
			return false;
		}
		if (!cl.empty()) 
		{
			setStatusCode(400);
			setState(ERRORE);
			return false;
		}
		_isChunked = true;
		_state = BODY_CHUNKED;
		_chunkState = CHUNK_SIZE;
	}
	return true;
}

bool HTTPRequest::validateMultipartFormData() 
{
	std::string ct = getHeaderValue("content-type");
	if (ct.find("multipart/form-data") == std::string::npos)
		return true;
	size_t pos = ct.find("boundary=");
	if (pos == std::string::npos) 
	{
		setStatusCode(469);
		setState(ERRORE);
		return false;
	}
	std::string boundary = ct.substr(pos + 9);
	if (boundary.empty()) 
	{
		setStatusCode(469);
		setState(ERRORE);
		return false;
	}
	_uploadBoundary = "--" + boundary;
	_multipartState = PART_HEADER;
	_state = BODY_MULTIPART;
	return true;
}

bool HTTPRequest::validateAllowedMethods() 
{
	if (_location && _location->isMethodAllowed(_method))
		return true;
	setState(ERRORE);
	setStatusCode(405);
	return false;
}

void HTTPRequest::clear() 
{
	if (_uploadFile.is_open())
		_uploadFile.close();
	if (_chunkFile.is_open())
		_chunkFile.close();
	_statusCode = 0;
	_state = INIT;
	_parsePosition = 0;
	_contentLength = 0;
	_method = "GET";
	_uri = "/";
	_path = "";
	_query = "";
	_protocol = "HTTP/1.1";
	_bodyBuffer = "";
	_headerKey = "";
	_headerValue = "";
	_headers.clear();
	_contentType = "";
	_boundary = "";
	_isChunked = false;
	_keepAlive = true;
	_chunkSize = -1;
	_multipartBoundary = "";
	_location = NULL;
	_uploadHeadersParsed = false;
	_chunkFileInitialized = false;
	_uploadFilepath = "";
	_uploadBoundary = "";
	_multipartState = PART_HEADER;
	_chunkState = CHUNK_SIZE;
	_hasCgi = false;
}
