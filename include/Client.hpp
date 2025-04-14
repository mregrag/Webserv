/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 16:06:48 by mregrag           #+#    #+#             */
/*   Updated: 2025/04/14 16:20:28 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <netinet/in.h>
#include <ctime>
#include "ServerConfig.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

# include <string>

class Client 
{
	private:
		int				_fd;
		std::string		_readBuffer;
		std::string		_writeBuffer;
		bool			_requestComplete;
		HTTPRequest		_request;
		size_t			_bytesSent;
		ServerConfig* _server;

	public:
		Client(int fd_client);
		~Client();

		int							getFd() const;
		const std::string&			getWriteBuffer() const;
		std::string&				getWriteBuffer();
		const std::string&			getReadBuffer() const;
		bool						isRequestComplete() const;
		HTTPRequest&				getRequest();
		size_t&						getBytesSent();

		void	appendToBuffer(const char* data, size_t length);
		void	parseRequest();
		void	setResponse(const std::string& response);
		void	clearBuffers();
		void setServer(ServerConfig* server); 
		ServerConfig* getServer() const;
};

#endif 
