/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 16:09:58 by mregrag           #+#    #+#             */
/*   Updated: 2025/04/10 22:25:49 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <map>

class HTTPRequest
{
	public:
		HTTPRequest();
		HTTPRequest(const HTTPRequest &other);
		HTTPRequest &operator=(const HTTPRequest &rhs);
		~HTTPRequest();

		// Parse a raw HTTP request string; returns true if successful.
		void parse();
		// Clear all internal data.
		void clear();

		// Getters for HTTP request components.
		const std::string &getMethod() const;
		const std::string &getUrl() const;
		const std::string &getVersion() const;
		const std::map<std::string, std::string> &getHeaders() const;
		const std::string &getBody() const;

	private:
		std::string _method;
		std::string _url;
		std::string _version;
		std::map<std::string, std::string> _headers;
		std::string _body;
};

#endif // HTTP_REQUEST_HPP

