#ifndef CLIENT_SESSION_HPP
#define CLIENT_SESSION_HPP

#include "webserver.hpp"

class ClientSession
{
public:
    ClientSession(int clientFd);
    ~ClientSession();

    bool handleRequest();  // Reads request data and parses it
    bool sendResponse();   // Sends response back to the client
    bool isComplete() const;  // Checks if session should be closed

private:
    int _clientFd; // Client socket
    HTTPRequest _request;
    HTTPResponse _response;
};


#endif