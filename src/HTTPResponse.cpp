#include "../include/HTTPResponse.hpp"
#include "../include/Utils.hpp"
#include "../include/webserver.hpp"

HTTPResponse::HTTPResponse(Client *client) : _client(client),  _request(_client->getRequest()),_statusCode(200),_statusMessage("OK"),_body("")
{
}

HTTPResponse::HTTPResponse(const HTTPResponse &rhs)
{
	*this = rhs;
}

HTTPResponse &HTTPResponse::operator=(const HTTPResponse &rhs)
{
	if (this != &rhs)
	{
		_client = rhs._client;
		_request = rhs._request;
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
	return _response;
}

int HTTPResponse::buildResponse(void)
{

	if (_request->getMethod() == "GET")
		handleGetRequest();
	else if (_request->getMethod() == "POST")
		handlePostRequest();
	else if (_request->getMethod() == "DELETE")
		handleDeleteRequest();

	return (0);
}


LocationConfig HTTPResponse::findMatchingLocation(const std::string& requestUri) 
{
	ServerConfig* config = _client->getServer();
	const std::map<std::string, LocationConfig>& locations = config->getLocations();

	std::string bestMatch = "";

	for (std::map<std::string, LocationConfig>::const_iterator it = locations.begin(); it != locations.end(); ++it) 
	{
		const std::string& locationPath = it->first;
		if (locationPath == requestUri)
			return it->second;
		if (requestUri.find(locationPath) == 0) 
			if (locationPath.length() > bestMatch.length()) 
				bestMatch = locationPath;
	}

	if (!bestMatch.empty()) 
		return locations.find(bestMatch)->second;

	std::map<std::string, LocationConfig>::const_iterator defaultLoc = locations.find("/");
	if (defaultLoc != locations.end()) 
		return defaultLoc->second;

	throw std::runtime_error("No matching location found for URI: " + requestUri);
}

void HTTPResponse::handleGetRequest(void)
{
	const std::string& requestUri = _client->getRequest()->getPath();
	LocationConfig location = findMatchingLocation(requestUri);

	if (!location.isMethodAllowed("GET")) 
		return(setStatusCode(405));

	std::string relativePath = requestUri.substr(location.getPath().length());
	std::string filePath = location.getRoot() + relativePath;

	if (!filePath.empty() && filePath[filePath.length() - 1] == '/')
		filePath += location.getIndex();

	std::ifstream file(filePath.c_str(), std::ios::in | std::ios::binary);

	if (!file.is_open())
	{
		std::string dirPath = location.getRoot() + relativePath;

		if (Utils::isDirectory(dirPath))
		{
			if (location.isAutoIndexOn())
			{
				std::string autoIndexPage = Utils::listDirectory(dirPath, location.getRoot(), requestUri);

				std::ostringstream response;
				response << "HTTP/1.1 200 OK\r\n"
					<< "Content-Length: " << autoIndexPage.length() << "\r\n"
					<< "Content-Type: text/html\r\n"
					<< "Connection: close\r\n"
					<< "\r\n"
					<< autoIndexPage;

				setResponse(response.str());
				return;
			}
		}
		setStatusCode(404);
		return;
	}
	std::stringstream bodyStream;
	bodyStream << file.rdbuf();
	file.close();

	std::string contentType = Utils::getMimeType(filePath);
	std::string body = bodyStream.str();

	std::ostringstream response;
	response << "HTTP/1.1 200 OK\r\n"
		<< "Content-Length: " << body.length() << "\r\n"
		<< "Content-Type: " << contentType << "\r\n"
		<< "Connection: close\r\n"
		<< "\r\n"
		<< body;

	setResponse(response.str());
}


void HTTPResponse::handlePostRequest(void)
{

}

void HTTPResponse::handleDeleteRequest(void)
{


}

void HTTPResponse::setResponse(const std::string& response)
{
	this->_response = response;
}
