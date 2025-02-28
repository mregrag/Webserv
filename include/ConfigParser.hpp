/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 17:16:39 by mregrag           #+#    #+#             */
/*   Updated: 2025/02/25 23:19:06 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <string>
#include <vector>
#include "../include/ServerConfig.hpp"

class ConfigParser
{
	private:
		std::vector<ServerConfig> _servers;
		std::string _configFilePath;

		// Private helper methods
		void parseFile();
		void parseServerBlock(const std::string& block);
		LocationConfig parseLocationBlock(const std::string& block);
		std::pair<std::string, std::string> parseLine(const std::string& line);
		std::string trim(const std::string& str);
		std::vector<std::string> split(const std::string& str, char delimiter);

	public:
		// Orthodox Canonical Form
		ConfigParser();
		ConfigParser(const std::string& configFilePath);
		ConfigParser(const ConfigParser& other);
		ConfigParser& operator=(const ConfigParser& other);
		~ConfigParser();

		// Public methods
		bool parse();
		const std::vector<ServerConfig>& getServers() const;
};

#endif 
