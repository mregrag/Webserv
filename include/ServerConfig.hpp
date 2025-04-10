/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zel-oirg <zel-oirg@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/24 21:24:19 by mregrag           #+#    #+#             */
/*   Updated: 2025/04/10 01:44:53 by zel-oirg         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include "LocationConfig.hpp"
#include <string>
#include <map>
#include <vector>
# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/select.h>
# include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <netdb.h>     // For getaddrinfo
#include <arpa/inet.h> // For inet_ntoa




class ServerConfig
{
	private:
		std::string								_host;
		uint16_t								_port;
		std::string 							_serverName;
		size_t									_clientMaxBodySize;
		std::map<int, std::string>				_errorPages;
		std::map<std::string, LocationConfig>	_locations;
		struct sockaddr_in						_server_address;
		int     				_listen_fd;

		//utils methode
		bool isValidHost(const std::string& host);

	public:
		ServerConfig();
		~ServerConfig();
		ServerConfig(const ServerConfig& other);
		ServerConfig&	operator=(const ServerConfig& other);

		void	setHost(const std::string& host);
		void	setPort(const std::string& port);
		void	setServerName(const std::string& name);
		void	setClientMaxBodySize(size_t size);
		void	setErrorPage(int code, const std::string& path);
		void	addLocation(const std::string& path, const LocationConfig& location);
		void	setFd(int fd);

		int	getPort() const;
		int getFd();
		size_t	getClientMaxBodySize() const;
		const std::string&	getServerName() const;
		const std::string&	getHost() const;
		const std::map<int, std::string>&	getErrorPages() const;
		const std::map<std::string, LocationConfig>&	getLocations() const;

		void print() const;
		  // Socket and server setup
		void setupServer();

		int acceptConnection();

		class ErrorServer : public std::exception
		{
			private:
				std::string _message;
			public:
				ErrorServer(std::string message) throw()
				{
					_message = "SERVER ERROR: " + message;
				}
				virtual const char* what() const throw()
				{
					return (_message.c_str());
				}
				virtual ~ErrorServer() throw() {}
		};



};

#endif
