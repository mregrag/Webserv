/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zel-oirg <zel-oirg@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/24 21:18:46 by mregrag           #+#    #+#             */
/*   Updated: 2025/03/20 07:30:08 by zel-oirg         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ConfigParser.hpp"
#include "../include/webserver.hpp"


int main(int argc, char **argv)
{
	if (argc != 1 && argc != 2)
	{
		std::cout << "Usage: " << argv[0] << " [config_file]\n";
		return (1);
	}

	try
	{
		std::string configFile = (argc == 1) ? "config/default.conf" : argv[1];

		ConfigParser config(configFile);
		if (!config.parse())
		{
			std::cout << "Failed to parse configuration file.\n";
			return (1);
		}
		std::vector<ServerConfig> serversConfig = config.getServers();
		ServerManager servers(serversConfig);
		servers.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << "\n";
		return (1);
	}
	return (0);
}

