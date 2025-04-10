/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 16:06:48 by mregrag           #+#    #+#             */
/*   Updated: 2025/04/10 22:21:17 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <netinet/in.h>
#include <ctime>
#include "ServerConfig.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

class Client
{
	public:
		Client();
		Client(const Client &other);
		Client(ServerConfig &config);
		Client &operator=(const Client &rhs);
		~Client();

		const int &getSocket() const;
		const struct sockaddr_in &getAddress() const;

		void setSocket(int socket);
		void setAddress(const struct sockaddr_in &addr);
		void setServer(const ServerConfig &config);
		void buildResponse();
		void clearClient();

		HTTPResponse response;
		HTTPRequest request;

		ServerConfig server;

	private:
		int _client_socket;
		struct sockaddr_in _client_address;
};

#endif 
