#include "../include/webserver.hpp"

HTTPRequest::HTTPRequest(const std::string& raw_data)
{
    _rawData = raw_data;
}

HTTPRequest::~HTTPRequest() {}

const std::string& HTTPRequest::getMethod() const
{
    return _method;
}

const std::string&  HTTPRequest::getPath() const
{
    return _path;
}

const std::string&  HTTPRequest::getHeader(const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator  it = _headers.find(key);
    if (it != _headers.end())
        return (*it).second;
    throw(std::out_of_range("Header " + key + " is out of range"));
}

int HTTPRequest::getContentLength() const
{
    return atoi(getHeader("Content-Length").c_str());
}

void    HTTPRequest::parce()
{
    std::string::size_type  end_header = _rawData.find("\r\n\r\n");
    std::string headers_data = _rawData.substr(0, end_header);
    _body = _rawData.substr(end_header + 4);
    std::istringstream  stream(headers_data);
    
    std::string request_line;
    getline(stream, request_line);
    std::istringstream  request_line_stream(request_line);
    request_line_stream >> _method >> _path >> _protocol;
    
    std::string line;
    while (getline(stream, line))
    {
        if (line.size() >= 2 && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1);
        std::string::size_type  colon = line.find(":");
        if (colon != std::string::npos)
        {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 1);
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            _headers[key] = value;
        }
    }
    std::cout << "method = " << _method << std::endl;
    std::cout << "path = " << _path << std::endl;
    std::cout << "protocol = " << _protocol << std::endl;
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
        std::cout << "Key: " << it->first << ", Value: " << it->second << "\n";
    }
}
