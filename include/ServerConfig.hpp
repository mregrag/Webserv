/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/24 21:24:19 by mregrag           #+#    #+#             */
/*   Updated: 2025/02/25 23:18:33 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <string>
#include <vector>
#include <map>
#include "../include/LocationConfig.hpp"

class ServerConfig
{
	private:
		int _port;
		std::string _host;
		std::string _serverName;
		size_t _clientMaxBodySize;
		std::map<int, std::string> _errorPages;
		std::vector<LocationConfig> _locations;

	public:
		// Orthodox Canonical Form
		ServerConfig();
		ServerConfig(const ServerConfig& other);
		ServerConfig& operator=(const ServerConfig& other);
		~ServerConfig();

		// Getters
		int getPort() const;
		const std::string& getHost() const;
		const std::string& getServerName() const;
		size_t getClientMaxBodySize() const;
		const std::map<int, std::string>& getErrorPages() const;
		const std::vector<LocationConfig>& getLocations() const;

		// Setters
		void setPort(int port);
		void setHost(const std::string& host);
		void setServerName(const std::string& serverName);
		void setClientMaxBodySize(size_t size);
		void addErrorPage(int code, const std::string& path);
		void addLocation(const LocationConfig& location);
};

#endif // SERVER_CONFIG_HPP

