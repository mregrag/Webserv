#include "../include/HTTPResponse.hpp"
#include "../include/Utils.hpp"
#include "../include/webserver.hpp"

// Constructor/Destructor
HTTPResponse::HTTPResponse(Client* client) : _client(client), _request(_client->getRequest()), _statusCode(200), _statusMessage("OK"), _body("") 
{
}

HTTPResponse::HTTPResponse(const HTTPResponse& rhs) 
{
	*this = rhs;
}

HTTPResponse::~HTTPResponse() 
{
}

HTTPResponse& HTTPResponse::operator=(const HTTPResponse& rhs) 
{
	if (this != &rhs) 
	{
		_client = rhs._client;
		_request = rhs._request;
		_statusCode = rhs._statusCode;
		_statusMessage = rhs._statusMessage;
		_headers = rhs._headers;
		_body = rhs._body;
		_response = rhs._response;
	}
	return *this;
}

// Public methods
void HTTPResponse::clear() 
{
	_statusCode = 200;
	_statusMessage = "OK";
	_headers.clear();
	_body.clear();
	_response.clear();
}

void HTTPResponse::setStatusCode(int code) 
{
	_statusCode = code;
}

void HTTPResponse::setStatusMessage(const std::string& message) 
{
	_statusMessage = message;
}

void HTTPResponse::setHeader(const std::string& key, const std::string& value) 
{
	_headers[key] = value;
}

void HTTPResponse::setBody(const std::string& body) 
{
	_body = body;
	std::ostringstream oss;
	oss << _body.length();
	setHeader("Content-Length", oss.str());
}

void HTTPResponse::setResponse(const std::string& response) 
{
	_response = response;
}

std::string HTTPResponse::getResponse() const 
{
	return _response;
}

void HTTPResponse::handlePostRequest() 
{
	buildErrorResponse(501, "Not Implemented");
}

void HTTPResponse::handleDeleteRequest() 
{
	buildErrorResponse(501, "Not Implemented");
}



int HTTPResponse::buildResponse() 
{
	const std::string& method = _request->getMethod();

	if (method == "GET") 
		handleGetRequest();
	else if (method == "POST") 
		handlePostRequest();
	else if (method == "DELETE") 
		handleDeleteRequest();
	else 
		buildErrorResponse(405, "Method Not Allowed");

	return 0;
}

LocationConfig HTTPResponse::findMatchingLocation(const std::string& requestUri) const 
{
	ServerConfig* config = _client->getServer();
	const std::map<std::string, LocationConfig>& locations = config->getLocations();
	std::string bestMatch;

	for (std::map<std::string, LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); ++it) 
	{
		if (it->first == requestUri) 
			return it->second;
		if (requestUri.find(it->first) == 0 && it->first.length() > bestMatch.length()) 
			bestMatch = it->first;
	}

	if (!bestMatch.empty()) 
		return locations.find(bestMatch)->second;

	std::map<std::string, LocationConfig>::const_iterator defaultLoc = locations.find("/");
	if (defaultLoc != locations.end()) 
		return defaultLoc->second;

	throw std::runtime_error("No matching location found for URI: " + requestUri);
}

std::string HTTPResponse::buildFullPath(const LocationConfig& location, const std::string& requestUri) const 
{
	std::string relativePath = requestUri.substr(location.getPath().length());
	std::string direPath = location.getRoot() + relativePath;

	if((!direPath.empty() && direPath[direPath.length() - 1] == '/') || Utils::isDirectory(direPath))
	{
		if (!direPath.empty() && direPath[direPath.length() - 1] != '/') 
			direPath += '/';

		std::string indexPath = direPath + location.getIndex();

		if (Utils::fileExists(indexPath)) 
			return (indexPath);
	}

	return (direPath);
}

void HTTPResponse::handleGetRequest() 
{
	try {
		const std::string& requestUri = _request->getPath();
		LocationConfig location = findMatchingLocation(requestUri);

		if (!location.isMethodAllowed("GET")) 
		{
			buildErrorResponse(405);
			return;
		}
		std::string fullPath = buildFullPath(location, requestUri);
		if (Utils::isDirectory(fullPath)) 
		{
			if (location.isAutoIndexOn()) 
			{
				std::string autoIndexContent = Utils::listDirectory(fullPath, location.getRoot(), requestUri);
				buildAutoIndexResponse(autoIndexContent);
				return;
			}
			buildErrorResponse(403);
			return;
		}
		// Handle regular file requests
		if (!Utils::fileExists(fullPath)) 
		{
			buildErrorResponse(404);
			return;
		}

		buildSuccessResponse(fullPath);

	} 
	catch (const std::exception& e) 
	{
		buildErrorResponse(500, e.what());
	}
}

std::string HTTPResponse::readFileContent(const std::string& filePath) const 
{
	std::ifstream file(filePath.c_str(), std::ios::binary);
	if (!file.is_open()) 
		throw std::runtime_error("Could not open file: " + filePath);

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

void  HTTPResponse::buildSuccessResponse(const std::string& fullPath)
{
	std::string fileContent = readFileContent(fullPath);
	std::ostringstream response;
	response << "HTTP/1.1 " << _statusCode << " " << _statusMessage << "\r\n"
		<< "Content-Length: " << fileContent.length() << "\r\n"
		<< "Content-Type: " << Utils::getMimeType(fullPath) << "\r\n"
		<< "Connection: close\r\n";

	response << "\r\n" << fileContent;
	this->setResponse(response.str());
}

void HTTPResponse::buildAutoIndexResponse(const std::string& autoIndexContent)
{
	std::ostringstream response;
	response << "HTTP/1.1 " << _statusCode << " " << _statusMessage << "\r\n"
		<< "Content-Length: " << autoIndexContent.length() << "\r\n"
		<< "Content-Type: " << "text/html" << "\r\n"
		<< "Connection: close\r\n";

	response << "\r\n" << autoIndexContent;
	this->setResponse(response.str());
}
void  HTTPResponse::buildErrorResponse(int statusCode, const std::string& message)
{
	
	ServerConfig* config = _client->getServer();
	const std::map<int, std::string>& error_pages = config->getErrorPages();

	std::map<int, std::string>::const_iterator it = error_pages.find(statusCode);
	std::string file_content = it->second;

	std::string fileContent = readFileContent(file_content);
	std::ostringstream response;
	std::string statusMsg = message.empty() ? "Error" : message;

	response << "HTTP/1.1 " << statusCode << " " << statusMsg << "\r\n"
		<< "Content-Type: " << Utils::getMimeType(file_content) << "\r\n"
		<< "Connection: close\r\n"
		<< "\r\n"
		<< fileContent;
	this->setResponse(response.str());
}
