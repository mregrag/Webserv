#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <ctime>
#include <netinet/in.h>
#include <sys/types.h>
#include "ServerConfig.hpp"
#include "HTTPRequest.hpp"
#include <cstring>
#include "HTTPResponse.hpp"


enum ClientState 
{
	CLIENT_READING,
	CLIENT_WRITING,
	CLIENT_ERROR
};

class HTTPRequest;
class HTTPResponse;
class CGIHandler;
class Client {
public:
	Client(int fd, ServerConfig* server);
	~Client();

	std::string& getReadBuffer();
	void setReadBuffer(const char* data, ssize_t bytesSet);
	void setClientAddress(const struct sockaddr_in& addr);
	ssize_t getResponseChunk(char* buffer, size_t bufferSize);
	ssize_t getHeaderResponse(char* buffer, size_t bufferSize);
	ssize_t getStringBodyResponse(char* buffer, size_t bufferSize);
	ssize_t getFileBodyResponse(char* buffer, size_t bufferSize);

	int getFd() const;
	time_t getLastActivity() const;
	void updateActivity();
	ClientState getState() const;
	HTTPRequest* getRequest();   // Returns a pointer to the embedded request object.
	HTTPResponse* getResponse(); // Returns a pointer to the embedded response object.
	ServerConfig* getServer() const;
	bool shouldKeepAlive() const;
	bool hasTimedOut(time_t currentTime, time_t timeout) const;
	void reset();

private:
	int _fd;
	ServerConfig* _server;
	HTTPRequest _request;      // Directly embedded HTTPRequest
	HTTPResponse _response;    // Directly embedded HTTPResponse
	int _bytesSent;
	size_t _headersSize;
	time_t _lastActivity;
	ClientState _state;
	std::string _readBuffer;
	struct sockaddr_in _addr;
	std::ifstream _fileStream; // Used for file-based response body
};

#endif // CLIENT_HPP
