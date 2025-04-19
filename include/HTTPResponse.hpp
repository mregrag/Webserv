/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 16:10:25 by mregrag           #+#    #+#             */
/*   Updated: 2025/04/18 21:52:15 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <string>
#include <map>
#include "../include/HTTPRequest.hpp"
#include "ServerConfig.hpp"
#include "../include/Client.hpp"


class HTTPResponse
{
	public:
		HTTPResponse(Client *client);
		HTTPResponse(const HTTPResponse &rhs);
		HTTPResponse &operator=(const HTTPResponse &rhs);
		~HTTPResponse();

		// Setters.
		void setStatusCode(int code);
		void setStatusMessage(const std::string &message);
		void setHeader(const std::string &key, const std::string &value);
		void setBody(const std::string &body);
		void setServer(const ServerConfig& server);
		// Clear all internal data.
		void clear();

		// Build and get the full HTTP response string.
		std::string getResponse() const;

		int 	buildResponse(void);
		void	handleGetRequest(void);
		void	handlePostRequest(void);
		void	handleDeleteRequest(void);
		void	setResponse(const std::string& response);
		LocationConfig findMatchingLocation(const std::string& requestUri);

	private:
		Client *_client;
		HTTPRequest *_request;
		std::string _response;
		int _statusCode;
		std::string _statusMessage;
		std::map<std::string, std::string> _headers;
		std::string _body;
};

#endif // HTTP_RESPONSE_HPP

