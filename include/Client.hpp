/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 16:06:48 by mregrag           #+#    #+#             */
/*   Updated: 2025/05/01 19:13:33 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <netinet/in.h>
#include <ctime>
#include "ServerConfig.hpp"

# include <string>
class HTTPResponse;
class HTTPRequest;
class Client {
	private:
		int _fd;                    // Client socket file descriptor
		ServerConfig* _server;      // Associated server configuration
		time_t _lastActivity;       // Last activity timestamp
		std::string _readBuffer;    // Buffer for incoming request data
		std::string _writeBuffer;   // Buffer for outgoing response data
		size_t _bytesSent;          // Bytes sent for current response
		HTTPRequest* _request;      // Request parser
		HTTPResponse* _response;    // Response generator

		// Private to prevent copying (avoids FD duplication)
		Client(const Client&);
		Client& operator=(const Client&);

	public:
		Client(int fd, ServerConfig* server);
		~Client();

		// Handle epoll events
		void handleRead();  // Process EPOLLIN (read request)
		void handleWrite(); // Process EPOLLOUT (send response)

		// Getters
		int getFd() const;
		time_t getLastActivity() const;
		ServerConfig* getServer() const;
		HTTPRequest* getRequest();
		HTTPResponse* getResponse();

		// Utility methods
		void updateActivity();
		void reset(); // Reset for keep-alive connections
		void clearBuffers();
};

#endif 
