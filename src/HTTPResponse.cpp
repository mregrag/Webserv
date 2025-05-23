#include "../include/HTTPResponse.hpp"
#include "../include/CGIHandler.hpp"
#include "../include/Client.hpp"
#include "../include/HTTPRequest.hpp"
#include "../include/Utils.hpp"
#include "../include/Logger.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <errno.h>
#include <ostream>
#include <string>
#include <strings.h>
#include <sys/wait.h>

HTTPResponse::HTTPResponse(HTTPRequest* request)
	: _request(request),
	_protocol("HTTP/1.1"),
	_statusCode(200),
	_statusMessage("OK"),
	_body(""),
	_header(""),
	_filePath(""),
	_fileSize(0),
	_state(INIT)
{
	if (!_request)
		throw std::runtime_error("Invalid HTTPRequest pointer in HTTPResponse constructor");
}

HTTPResponse::HTTPResponse(const HTTPResponse& rhs) {
	*this = rhs;
}

HTTPResponse& HTTPResponse::operator=(const HTTPResponse& rhs) {
	if (this != &rhs) {
		_request       = rhs._request;
		_protocol      = rhs._protocol;
		_statusCode    = rhs._statusCode;
		_statusMessage = rhs._statusMessage;
		_headers       = rhs._headers;
		_body          = rhs._body;
		_header        = rhs._header;
		_filePath      = rhs._filePath;
		_fileSize      = rhs._fileSize;
		_state         = rhs._state;
	}
	return *this;
}

HTTPResponse::~HTTPResponse() {
	// No additional cleanup required; containers clean themselves.
}

void HTTPResponse::clear() {
	_protocol      = "HTTP/1.1";
	_statusCode    = 200;
	_statusMessage = "OK";
	_headers.clear();
	_body.clear();
	_header.clear();
	_filePath.clear();
	_fileSize      = 0;
	_state         = INIT;
}

void HTTPResponse::setProtocol(const std::string& protocol) {
	_protocol = protocol;
}

void HTTPResponse::setStatusCode(int code) {
	_statusCode = code;
}

void HTTPResponse::setStatusMessage(const std::string& message) {
	_statusMessage = message;
}

void HTTPResponse::setHeader(const std::string& key, const std::string& value) {
	_headers[key] = value;
}

void HTTPResponse::appendToBody(const std::string& bodyPart) {
	_body.append(bodyPart);
}

void HTTPResponse::setBodyResponse(const std::string& filePath) {
	_filePath = filePath;
	struct stat file_stat;
	if (stat(filePath.c_str(), &file_stat) == -1) {
		LOG_ERROR("Failed to stat file " + filePath + ": " + std::string(strerror(errno)));
		throw std::runtime_error("Failed to stat file: " + filePath);
	}
	_fileSize = file_stat.st_size;
}

void HTTPResponse::setState(response_state state) {
	_state = state;
}

std::string HTTPResponse::getHeader() const {
	return _header;
}

std::string HTTPResponse::getBody() const {
	return _body;
}

std::string HTTPResponse::getFilePath() const {
	return _filePath;
}

size_t HTTPResponse::getContentLength() const {
	return !_body.empty() ? _body.size() : _fileSize;
}

size_t HTTPResponse::getFileSize() const {
	return _fileSize;
}

int HTTPResponse::getState() const {
	return _state;
}

bool HTTPResponse::shouldCloseConnection() const {
	// Retrieve the connection header from _request.
	int reqStatus = _request->getStatusCode();
	std::string connection = Utils::trim(_request->getHeaderValue("Connection"));

	if (connection == "close")
		return true;
	if (connection == "keep-alive")
		return false;
	// For certain status codes, connection should close.
	if (reqStatus == 400 || reqStatus == 408 || reqStatus == 414 ||
		reqStatus == 431 || reqStatus == 501 || reqStatus == 505)
		return true;
	return false;
}

void HTTPResponse::buildHeader() {
	std::ostringstream oss;
	oss << _protocol << " " << _statusCode << " " << _statusMessage << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin();
	it != _headers.end(); ++it)
	{
		oss << it->first << ": " << it->second << "\r\n";
	}
	oss << "\r\n";
	_header = oss.str();
}

void HTTPResponse::buildResponse() 
{
	if (_request->getState() == ERRORE)
		buildErrorResponse(_request->getStatusCode());
	else if (_request->getLocation() && _request->getLocation()->hasRedirection())
		handleRedirect();
	else if (_request->getMethod() == "GET")
		handleGet();
	else if (_request->getMethod() == "POST")
		handlePost();
	else if (_request->getMethod() == "DELETE")
		handleDelete();
	else
		buildErrorResponse(405);
}

void HTTPResponse::buildErrorResponse(int statusCode) 
{
	std::string defaultPage = _request->getServer()->getErrorPage(statusCode);
	setProtocol(_request->getProtocol());
	setStatusCode(statusCode);
	setStatusMessage(Utils::getMessage(statusCode));
	setHeader("Server", "1337webserver");
	setHeader("Date", Utils::getCurrentDate());
	setHeader("Content-Type", "text/html");

	if (Utils::fileExists(defaultPage)) {
		setBodyResponse(defaultPage);
		setHeader("Content-Length", Utils::toString(getFileSize()));
	} else {
		appendToBody(defaultPage);
		setHeader("Content-Length", Utils::toString(_body.size()));
	}
	setHeader("Connection", shouldCloseConnection() ? "close" : "keep-alive");
	buildHeader();
	setState(HEADER_SENT);
}

void HTTPResponse::buildSuccessResponse(const std::string& fullPath) {
	setProtocol(_request->getProtocol());
	setStatusCode(_request->getStatusCode());
	setStatusMessage(Utils::getMessage(_request->getStatusCode()));
	setHeader("Server", "1337webserver");
	setHeader("Date", Utils::getCurrentDate());
	setHeader("Connection", shouldCloseConnection() ? "close" : "keep-alive");
	setHeader("Content-Type", Utils::getMimeType(fullPath));
	setBodyResponse(fullPath);
	setHeader("Content-Length", Utils::toString(getContentLength()));
	buildHeader();
	setState(FINISH);
}

void HTTPResponse::handleCGIRequest() 
{
	LOG_INFO("cgi request came in");

	CGI	cgi(*_request);
	cgi.init();
	cgi.execute();
	
	// appendToBody("output");
	// setProtocol(_request->getProtocol());
	// setStatusCode(_request->getStatusCode());
	// setStatusMessage(Utils::getMessage(_request->getStatusCode()));
	// setHeader("Server", "1337webserver");
	// setHeader("Date", Utils::getCurrentDate());
	// setHeader("Connection", shouldCloseConnection() ? "close" : "keep-alive");
	// setHeader("Content-Type", "text/html");
	// setHeader("Content-Length", Utils::toString(getContentLength()));
	// buildHeader();
	// setState(FINISH);
}

void HTTPResponse::handleGet() 
{
	std::string resource = _request->getResource();
	if (resource.empty()) 
	{
		buildErrorResponse(404);
		return;
	}
	else if (_request->isCgiRequest()) //?
	{
		handleCGIRequest();
	}
	else if (_request->getLocation() && _request->getLocation()->isAutoIndexOn() && Utils::isDirectory(resource)) 
	{
		handleAutoIndex();
		return;
	}
	else if (Utils::fileExists(resource)) 
	{
		buildSuccessResponse(resource);
		return;
	}
	else 
	{
		buildErrorResponse(403);
		return;
	}
}

void HTTPResponse::handlePost() 
{
	std::string resource = _request->getResource();
	if (resource.empty() || !Utils::fileExists(resource)) 
	{
		buildErrorResponse(404);
		return;
	}
	std::string body = "Resource created successfully\n";
	setProtocol(_request->getProtocol());
	setStatusCode(201);
	setStatusMessage("Created");
	setHeader("Content-Type", "text/plain");
	setHeader("Server", "1337webserver");
	setHeader("Date", Utils::getCurrentDate());
	setHeader("Connection", shouldCloseConnection() ? "close" : "keep-alive");
	appendToBody(body);
	setHeader("Content-Length", Utils::toString(_body.size()));
	buildHeader();
	setState(FINISH);
}

void HTTPResponse::handleDelete() {
	std::string resource = _request->getResource();
	if (resource.empty() || !Utils::fileExists(resource)) {
		buildErrorResponse(404);
		return;
	}
	if (Utils::isDirectory(resource)) {
		buildErrorResponse(403);
		return;
	}
	if (std::remove(resource.c_str()) != 0) {
		LOG_ERROR("Failed to delete file " + resource + ": " + std::string(strerror(errno)));
		buildErrorResponse(500);
		return;
	}
	setProtocol(_request->getProtocol());
	setStatusCode(200);
	setStatusMessage("OK");
	setHeader("Content-Type", "text/plain");
	setHeader("Server", "1337webserver");
	setHeader("Date", Utils::getCurrentDate());
	setHeader("Connection", shouldCloseConnection() ? "close" : "keep-alive");
	appendToBody("Resource deleted successfully\n");
	setHeader("Content-Length", Utils::toString(_body.size()));
	buildHeader();
	setState(HEADER_SENT);
}

void HTTPResponse::handleRedirect() 
{
	const LocationConfig* location = _request->getLocation();
	if (!location || !location->hasRedirection()) {
		buildErrorResponse(500);
		return;
	}
	int code = location->getRedirectCode();
	std::string reason = Utils::getMessage(code);
	std::string target = location->getRedirectPath();
	setProtocol(_request->getProtocol());
	setStatusCode(code);
	setStatusMessage(reason);
	setHeader("Content-Type", "text/html");
	setHeader("Location", target);
	setHeader("Server", "1337webserver");
	setHeader("Date", Utils::getCurrentDate());
	setHeader("Connection", shouldCloseConnection() ? "close" : "keep-alive");

	appendToBody("<html><head><title>");
	appendToBody(Utils::toString(code));
	appendToBody(" ");
	appendToBody(reason);
	appendToBody("</title></head><body><h1>");
	appendToBody(Utils::toString(code));
	appendToBody(" ");
	appendToBody(reason);
	appendToBody("</h1><hr><center>");
	appendToBody("1337webserver");
	appendToBody("</center></body></html>");

	setHeader("Content-Length", Utils::toString(_body.size()));
	buildHeader();
	setState(HEADER_SENT);
}

void HTTPResponse::handleAutoIndex() 
{
	std::string directory_path = _request->getResource();
	if (!Utils::isDirectory(directory_path)) 
	{
		buildErrorResponse(404);
		return;
	}
	std::vector<std::string> entries = Utils::listDirectory(directory_path);
	std::string req_path = _request->getPath();
	if (req_path.empty() || req_path[req_path.size()-1] != '/')
		req_path += '/';
	// Build an HTML directory listing.
	appendToBody("<!DOCTYPE html><html><head>");
	appendToBody("<title>Index of ");
	appendToBody(req_path);
	appendToBody("</title></head><body><h1>Index of ");
	appendToBody(req_path);
	appendToBody("</h1><hr><pre>");
	if (req_path != "/")
		appendToBody("<a href=\"../\">../</a>\n");
	for (size_t i = 0; i < entries.size(); ++i) 
	{
		std::string full_path = directory_path + "/" + entries[i];
		bool is_dir = Utils::isDirectory(full_path);
		std::string link_name = entries[i] + (is_dir ? "/" : "");
		appendToBody("<a href=\"" + req_path + link_name + "\">" + link_name + "</a>\n");
	}
	appendToBody("</pre><hr></body></html>");
	setProtocol(_request->getProtocol());
	setStatusCode(200);
	setStatusMessage("OK");
	setHeader("Content-Type", "text/html");
	setHeader("Content-Length", Utils::toString(_body.size()));
	buildHeader();
}
