#include "../include/HTTPResponse.hpp"
#include "../include/Utils.hpp"
#include "../include/Logger.hpp"
#include "../include/webserver.hpp"
#include <iostream>

HTTPResponse::HTTPResponse(Client* client) : _client(client), _request(_client->getRequest()), _statusCode(200), _statusMessage("OK"), _body(""), _state(INIT)
{
}

HTTPResponse::HTTPResponse(const HTTPResponse& rhs) 
{
	*this = rhs;
}

HTTPResponse::~HTTPResponse() 
{
}

void HTTPResponse::setProtocol(const std::string& protocol)
{
	_protocol = protocol;
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

void HTTPResponse::setResponse(/*const std::string& response*/) 
{
	std::ostringstream response;
	response << _protocol << " " << _statusCode << " " << _statusMessage << "\r\n";
	for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end();++it)	
		response << it->first << ": " << it->second << "\r\n";
	response << "\r\n";
	response << _body;
	_response = response.str();
	setState(FINISH);
}

std::string HTTPResponse::getResponse() const 
{
	return _response;
}

void HTTPResponse::handlePost() {}
void HTTPResponse::handleDelete() {}


int HTTPResponse::buildResponse()
{	
	// don t forget to handel non-finish request if you have to.
	if (is_req_well_formed())
		return -1;
	get_matched_location_for_request_uri(_request->getPath());
	if (this->_request->getMatchedLocation()->hasRedirection())
	{
		std::string Path;
		Path = _request->getFinalLocation()->getRoot() + _request->getFinalLocation()->getPath() + "/" + _request->getFinalLocation()->getIndex();
		buildSuccessResponse(Path);
		return 1;
	}
	if (!_hasMatchedLocation)
	{
		buildErrorResponse(404 ,"Not Found");
		return -1;
	}
	if (!_matched_location.isMethodAllowed(_request->getMethod()))
	{
		buildErrorResponse(405, "Method Not Allowed");
		return -1;
	}
	if (_request->getMethod() == "GET")
		handleGet(_matched_location);
	else if (_request->getMethod() == "POST")
		handlePost();
	else if (_request->getMethod() == "DELETE")
		handleDelete();
	
	return 1;
}

void HTTPResponse::get_matched_location_for_request_uri(const std::string& requestUri) 
{
	std::string best_match;
	std::map<std::string, LocationConfig> map_locations = _client->getServer()->getLocations();

	for (std::map<std::string, LocationConfig>::iterator it = map_locations.begin()
		; it != map_locations.end();it++)
	{
		if (it->first == requestUri)
		{
			_hasMatchedLocation = true;
			_matched_location = it->second ;
			return ;
		}
		if (requestUri.find(it->first) == 0)// /foo /foobar
		{
			if (requestUri.size() == it->first.size() || requestUri[it->first.size() - 1] == '/')
			{
				if (best_match.size() < it->first.size())
				{
					best_match = it->first;
					_hasMatchedLocation = true;
					_matched_location = it->second;
				}
			}
		}
	}
	// _matched_location.print();
}

void HTTPResponse::handleGet(LocationConfig location)
{
	std::string resource = get_requested_resource(location);
	std::cout << "get_requested_resource: " << resource << std::endl;
	if (access(resource.c_str(), F_OK) == 0)
	{
		if (get_resource_type(resource))// true if it is a regular file
		{
			if (location.if_location_has_cgi())
			{
				// code of cgi
			}
			else
				buildSuccessResponse(resource);
		}
		else// is a directory
		{
			if (is_uri_has_backslash_in_end(resource))
			{
				if (is_dir_has_index_files(resource, location.getIndex()))
				{
					if (location.if_location_has_cgi())
					{
						// run the cgi
					}
					else
						buildSuccessResponse(resource + location.getIndex());
				}
				else
				{
					if (location.getAutoindex())
					{
						/*
							resource is a directory that exist and have backslash at the end and there
							is no index file in the location of it
						*/
						std::vector<std::string> ls;
						DIR	*dir = opendir(resource.c_str());
						if (dir == NULL)
						{
							if (access(resource.c_str(), R_OK | X_OK))
								buildErrorResponse(403, "Forbidden");
							else
								buildErrorResponse(500, "Internal Server Error");
							return ;
						}
						struct dirent* entry = readdir(dir);
						while ((entry = readdir(dir)) != NULL)
						{
							if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..")
								continue ;
							ls.push_back(entry->d_name);
						}
						buildAutoIndexResponse(ls, _request->getPath()); 
					}
					else
						buildErrorResponse(403 , "Forbidden");
				}
			}
			else
			{
				// make a 301 redirection to request
				// uri with “/” addeed at the end
			}
		}
	}
	else
		buildErrorResponse(404, "Not Found");
}


void HTTPResponse::setState(response_state state)
{
	if (this->_state == FINISH)
		return (LOG_DEBUG("[setState (response)] Response already finished"));
	if (this->_state == state)
		return (LOG_DEBUG("[setState (response)] Response already in this state"));

	this->_state = state;
}

// new

bool HTTPResponse::is_req_well_formed()
{
	std::string encoding = _request->getHeaderValue("Transfer-Encoding");
	std::string content_lenght = _request->getHeaderValue("Content-Length");
	if (!encoding.empty() && encoding != "chunked")
	{
		buildErrorResponse(501 ,"Not Implemented");
		return true;
	}
	if (encoding.empty() && content_lenght.empty() && _request->getMethod() == "POST")
	{
		buildErrorResponse(400, "Bad Request");
		return true;
	}
	std::string uri = _request->getUri();
	size_t index = uri.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~:/?#[]@!$&'()*+,;=");
	if (index != std::string::npos)
	{
		buildErrorResponse(400, "Bad Request");
		return true;
	}
	if (uri.size() > 2048)
	{
		buildErrorResponse(414, "Request-URI Too Long");
		return true;
	}
	if (_request->getBody().size() > _client->getServer()->getClientMaxBodySize() )
	{
		buildErrorResponse(413, "Request Entity Too Large");
		return true;
	}
	return false;
}

const std::string HTTPResponse::get_requested_resource(LocationConfig location)
{
	std::string	root = location.getRoot() ;
	std::string	path = _request->getPath();
	size_t	a = 0;

	for (size_t i = 0; i < path.size(); i++)
	{
		if (path[i] == '/' && i + 1 < path.size())
			a = i;
	}
	path = path.substr(a + 1, path.size());
	return root + path;
}

bool HTTPResponse::get_resource_type(std::string resource)
{
    struct stat path_stat;
	stat(resource.c_str(), &path_stat);
	if (S_ISDIR(path_stat.st_mode))
		return false; // is a diractory
	return true;
}

bool HTTPResponse::is_uri_has_backslash_in_end(const std::string& resource)
{
	if (resource.empty())
		return false;
	if (resource[resource.size() - 1] == '/')
		return true;
	return false;
}

bool HTTPResponse::is_dir_has_index_files(const std::string& resource, const std::string& index)
{
	//TODO this is not good
	std::string	fullPath = resource + index;
	if (access(fullPath.c_str(), F_OK) == 0 && !index.empty())
		return true;
	return false;
}

//

std::string HTTPResponse::readFileContent(const std::string& filePath) const 
{
	std::ifstream file(filePath.c_str(), std::ios::binary);
	if (!file.is_open()) 
		throw std::runtime_error("Could not open file: " + filePath);

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

#include <sstream>
void  HTTPResponse::buildSuccessResponse(const std::string& fullPath)
{
	std::string fileContent = readFileContent(fullPath);
	std::ostringstream response;

	setStatusCode(_statusCode);
	setStatusMessage(_statusMessage);
	setProtocol("HTTP/1.1");
	setHeader("Content-Type", Utils::getMimeType(fullPath));
	setHeader("Connection", "keep-alive"); // keep-alive
	setBody(fileContent);
	// response << "HTTP/1.1 " << _statusCode << " " << _statusMessage << "\r\n"
	// 	<< "Content-Length: " << fileContent.length() << "\r\n"
	// 	<< "Content-Type: " << Utils::getMimeType(fullPath) << "\r\n"
	// 	<< "Connection: close\r\n";

	// response << "\r\n" << fileContent;
	this->setResponse();
}

void HTTPResponse::buildAutoIndexResponse(const std::vector<std::string>& list, const std::string& path) 
{
	std::stringstream	ss;
	
	ss	<< "<!DOCTYPE html>"
		<< "<html>"
		<< "<head>"
		<< "<title>Index of "<< path << "</title>"
		<< "</head>"
		<< "<body>"
		<< "<h1>Index of " << path <<" </h1>"
		<< "<ul>"
		<<  "<li><a href=\"../\">../</a></li>";
	for (std::vector<std::string>::const_iterator it = list.begin(); it != list.end(); it++)
		ss << "<li><a href=\"" << *it << "\">" << *it << "</a></li>";
	ss	<< "</ul>"
		<< "</body>"
		<< "</html>";
	std::string body;
	body = ss.str();
	setStatusCode(_statusCode);
	setProtocol("HTTP/1.1");
	setStatusMessage(_statusMessage);
	setHeader("Content-Type", "text/html");
	setHeader("Connection", "close");
	setBody(body);
	setResponse();
}

void  HTTPResponse::buildErrorResponse(int statusCode, const std::string& message)
{

	ServerConfig* config = _client->getServer();
	const std::map<int, std::string>& error_pages = config->getErrorPages();

	std::map<int, std::string>::const_iterator it = error_pages.find(statusCode);
	std::string file_name = it->second;

	std::string fileContent = readFileContent(file_name);
	std::string statusMsg = message.empty() ? "Error" : message;
	
	setStatusCode(statusCode);
	setStatusMessage(message);
	setProtocol("HTTP/1.1");
	setHeader("Content-Type", Utils::getMimeType(file_name));
	setHeader("Connection", "close");
	setBody(fileContent);
	
	this->setResponse();
}
