/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/24 21:18:46 by mregrag           #+#    #+#             */
/*   Updated: 2025/03/19 01:57:39 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ConfigParser.hpp"


int main(int argc, char **argv)
{
	if (argc != 1 && argc != 2)
	{
		std::cout << "Usage: " << argv[0] << " [config_file]\n";
		return (1);
	}

	try
	{
		std::string configFile = (argc == 1) ? "config/config.conf" : argv[1];

		ConfigParser config(configFile);
		if (!config.parse())
		{
			std::cout << "Failed to parse configuration file.\n";
			return (1);
		}
		std::vector<ServerConfig> servers = config.getServers();
		for (std::vector<ServerConfig>::iterator it = servers.begin(); it != servers.end(); ++it)
		{
			it->setupServer();
			it->startServer();
		}
		/*config.print();*/

		/*ServerManager server;*/
		/*server.setupServer(config.getServers());*/
		/*server.runServer();*/
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << "\n";
		return (1);
	}
	return (0);
}

