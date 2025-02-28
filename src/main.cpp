/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/24 21:18:46 by mregrag           #+#    #+#             */
/*   Updated: 2025/02/25 23:47:34 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ConfigParser.hpp"
#include <iostream>

// main.cpp - Example usage of Configuration Parser
#include "../include/ServerConfig.hpp"
#include <iostream>
#include <string>

void printServerConfig(const ServerConfig& config)
{
	std::cout << "Server Configuration:" << std::endl;
	std::cout << "Port: " << config.getPort() << std::endl;
	std::cout << "Host: " << config.getHost() << std::endl;
	std::cout << "Server Name: " << config.getServerName() << std::endl;
	std::cout << "Client Max Body Size: " << config.getClientMaxBodySize() << std::endl;

	std::cout << "\nError Pages:" << std::endl;
	const std::map<int, std::string>& errorPages = config.getErrorPages();
	std::map<int, std::string>::const_iterator it;
	for (it = errorPages.begin(); it != errorPages.end(); ++it)
		std::cout << "  " << it->first << ": " << it->second << std::endl;

	std::cout << "\nLocations:" << std::endl;
	const std::vector<LocationConfig>& locations = config.getLocations();
	for (size_t i = 0; i < locations.size(); ++i) {
		const LocationConfig& loc = locations[i];
		std::cout << "  Location: " << loc.getPath() << std::endl;
		std::cout << "    Root: " << loc.getRoot() << std::endl;
		std::cout << "    Index: " << loc.getIndex() << std::endl;
		std::cout << "    Autoindex: " << (loc.getAutoindex() ? "on" : "off") << std::endl;

		std::cout << "    Allowed Methods: ";
		const std::vector<std::string>& methods = loc.getAllowedMethods();
		for (size_t j = 0; j < methods.size(); ++j) {
			std::cout << methods[j];
			if (j < methods.size() - 1) {
				std::cout << ", ";
			}
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

int main(int argc, char* argv[])
{
	if (argc == 2 || argc == 1)
	{
		std::string configPath;
		configPath = (argc == 1? "config/default.conf" : argv[1]);
		ConfigParser parser(configPath);

		if (!parser.parse()) {
			std::cerr << "Failed to parse configuration file." << std::endl;
			return 1;
		}
		const std::vector<ServerConfig>& servers = parser.getServers();
		std::cout << "Parsed " << servers.size() << " server configurations." << std::endl;

		for (size_t i = 0; i < servers.size(); ++i) {
			printServerConfig(servers[i]);
		}
	}


	return 0;
}
