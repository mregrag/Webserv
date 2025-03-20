#ifndef CLIENT_SESSION_HPP
#define CLIENT_SESSION_HPP



class ClientSession
{
public:
    ClientSession(int clientFd, const ServerConfig& config);
    ~ClientSession();

    bool handleRequest();  // Reads request data and parses it
    bool sendResponse();   // Sends response back to the client
    bool isComplete() const;  // Checks if session should be closed

private:
    int _clientFd; // Client socket
    ServerConfig _config;
    // HTTPRequest _request;
    // HTTPResponse _response;
};


#endif