#include "../include/webserver.hpp"

ClientSession::ClientSession(int clientFd)
{
    _clientFd = clientFd;
}

ClientSession::~ClientSession() {}

bool ClientSession::handleRequest()
{
    char   buff[1000];
    ssize_t size = recv(_clientFd, buff, sizeof (buff), 0); 
    if (size == -1)
        throw(std::runtime_error("recv!"));
    _request.appendRawData(buff);
    _request.parse();
    return false;
}

bool ClientSession::sendResponse()
{
    return false;
}

bool ClientSession::isComplete() const
{
    return false;
}