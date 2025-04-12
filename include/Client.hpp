#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "webserver.hpp"

class Client
{
	private:
		int								_fd;
		std::string						_writeBuffer;
		std::string						_readBuffer;
		HTTPRequest						_request;
		bool							_requestComplete;
	public:
	Client(int fd);
	~Client();
	
	int	getFd() const;
	const std::string& getWriteBuffer() const;

	bool isRequestComplete() const;
	void appendToBuffer(const char* data, size_t length);
	void parseRequest();
	void prepareResponse(); // You’ll generate response here
	void clearWriteBuffer();
};
	

#endif 