/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 16:09:58 by mregrag           #+#    #+#             */
/*   Updated: 2025/04/14 00:12:34 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <cstddef>
#include <string>
#include <map>

class HTTPRequest
{
    private:
        std::map<std::string, std::string>				_headers;
        std::string							_rawData;
        std::string							_method;
        std::string							_path;
        std::string							_protocol;
        std::string							_body;
        bool								_complete;

    public:
        HTTPRequest(const std::string& raw_data);
        HTTPRequest();
        ~HTTPRequest();

        const std::string&	getMethod() const;
        const std::string&	getPath() const;
        const std::string&	getHeader(const std::string& key) const;
        size_t	getContentLength() const;

        void	parse();
        bool	check_request();
        void	write();
        bool	isComplete() const;
};



#endif // HTTP_REQUEST_HPP

