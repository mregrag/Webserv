#include "../include/webserver.hpp"


void    HTTPRequest::appendRawData(std::string str)
{
    _rawData += str;
}