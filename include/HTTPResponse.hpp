/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 16:10:25 by mregrag           #+#    #+#             */
/*   Updated: 2025/04/23 19:14:12 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "Client.hpp"
#include "LocationConfig.hpp"
#include "HTTPRequest.hpp"
#include <string>
#include <map>

class HTTPResponse 
{
	public:
		enum response_state
		{
			INIT,
			PROCESS,
			CHUNK,
			FINISH
		};
		HTTPResponse(Client* client);
		HTTPResponse(const HTTPResponse& rhs);
		~HTTPResponse();

		HTTPResponse& operator=(const HTTPResponse& rhs);

		int buildResponse();
		void clear();

		void setStatusCode(int code);
		void setStatusMessage(const std::string& message);
		void setHeader(const std::string& key, const std::string& value);
		void setBody(const std::string& body);
		void setResponse(const std::string& response);
		void setState(response_state state);
		int getState() const { return _state; }



		std::string getResponse() const;


	private:
		void handleGetRequest();
		void handlePostRequest();
		void handleDeleteRequest();

		LocationConfig findMatchingLocation(const std::string& requestUri) const;

		std::string buildFullPath(const LocationConfig& location, const std::string& requestUri) const;
		std::string readFileContent(const std::string& filePath) const;

		void buildSuccessResponse(const std::string& fullPath);
		void buildAutoIndexResponse(const std::string& autoIndexContent);
		void buildErrorResponse(int statusCode);

		Client* _client;
		HTTPRequest* _request;
		int _statusCode;
		std::string _statusMessage;
		std::map<std::string, std::string> _headers;
		std::string _body;
		std::string _response;
		response_state	_state;
};

#endif // HTTPRESPONSE_HPP
