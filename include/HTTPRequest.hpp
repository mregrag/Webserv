#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "../include/webserver.hpp"

class HTTPRequest
{
private:
    std::map<std::string, std::string>  _headers;
    std::string                         _rawData;
    std::string                         _method;
    std::string                         _path;
    std::string                         _protocol;
    std::string                         _body;
    bool                                _complete;
public:
    HTTPRequest(const std::string& raw_data);
    ~HTTPRequest();

    const std::string&  getMethod() const;
    const std::string&  getPath() const;
    const std::string&  getHeader(const std::string& key) const;
    int getContentLength() const;

    void    parce();
};


#endif