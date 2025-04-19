/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 16:09:58 by mregrag           #+#    #+#             */
/*   Updated: 2025/04/18 14:31:23 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <cstddef>
#include <string>
#include <map>

class Client;

class HTTPRequest
{

	public:
		enum	e_parse_state
		{
			INIT,
			LINE_METHOD,
			LINE_PATH,
			LINE_VERSION,
			LINE_END,
			HEADERS_INIT,
			HEADERS_KEY,
			HEADERS_VALUE,
			HEADERS_END,
			BODY_INIT,
			BODY_PROCESS,
			BODY_END,
			FINISH
		};
		HTTPRequest(Client *Client);
		~HTTPRequest();

		const std::string&	getMethod() const;
		const std::string&	getPath() const;
		const std::string&	getHeaderValue(const std::string& key) const;

		void	parseRequestLine(void);
		void	parseRequestHeader(void);
		void	parseRequestBody(void);


		void parseMethod(void);
		void parseUri(void);
		void parseVersion(void);
		void checkLineEnd(void);
		void parseHeaderKey();
		void parseHeaderValue();
		void parseHeaderEnd();

		void	parse(const std::string& rawdata);

		void	setState(e_parse_state state);
		int	getState(void);

		void	setStatusCode(int code);
		int 	getStatusCode(void);
		int urlDecode(std::string& str);
		void checkHeaderEnd();
	private:

		std::string							_request;
		std::map<std::string, std::string>				_headers;
		std::string							_method;
		std::string							_currentKey;
		std::string							_uri;
		std::string							_path;
		std::string							_query;
		std::string							_protocol;
		std::string							_body;
		std::string 							_currentHeaderValue;
		std::string							_currentHeaderKey;
		std::string							_tmpHeaderKey;
		std::string							_tmpHeaderValue;
		Client								*_client;
		int							_statusCode;
		e_parse_state						_state;
		size_t							_parsePosition;
		size_t							_contentLength;


};

#endif // HTTP_REQUEST_HPP
