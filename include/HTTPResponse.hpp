/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 16:10:25 by mregrag           #+#    #+#             */
/*   Updated: 2025/04/10 20:30:19 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <string>
#include <map>

class HTTPResponse
{
	public:
		HTTPResponse();
		HTTPResponse(const HTTPResponse &other);
		HTTPResponse &operator=(const HTTPResponse &rhs);
		~HTTPResponse();

		// Setters.
		void setStatusCode(int code);
		void setStatusMessage(const std::string &message);
		void setHeader(const std::string &key, const std::string &value);
		void setBody(const std::string &body);
		// Clear all internal data.
		void clear();

		// Build and get the full HTTP response string.
		std::string getResponse() const;

	private:
		int _statusCode;
		std::string _statusMessage;
		std::map<std::string, std::string> _headers;
		std::string _body;
};

#endif // HTTP_RESPONSE_HPP

