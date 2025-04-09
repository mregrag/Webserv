#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include "webserver.hpp"

class HTTPRequest {
    public:
        HTTPRequest();
        ~HTTPRequest();
    
        bool parse(const std::string& rawData);
        bool isComplete() const;
        bool keepAlive() const;
    
        const std::string& getMethod() const;
        const std::string& getPath() const;
        const std::string& getHeader(const std::string& key) const;
        size_t getContentLength() const;

        void    appendRawData(std::string str);
    private:
        std::string                         _rawData;
        std::map<std::string, std::string>  _headers;
        std::string                         _method;
        std::string                         _path;
        std::string                         _body;
        bool                                _complete;
    };

#endif