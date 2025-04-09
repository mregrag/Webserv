/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 17:16:39 by mregrag           #+#    #+#             */
/*   Updated: 2025/04/09 21:56:01 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include "webserver.hpp"
#include <string>

class ConfigParser
{
	private:
		ConfigFile _configFile;
		std::string _content;
		std::vector<ServerConfig> _servers;

		void validateConfigFile();
		void readAndPrepareContent();
		void parseServerBlocks();
		size_t findNextServerKeyword(size_t startPos);
		size_t findOpeningBrace(size_t startPos);
		std::pair<std::string, size_t> extractBlock(size_t openBracePos);
		void parseServerBlock(const std::string& block);
		LocationConfig parseLocationBlock(const std::string& block);
		std::pair<std::string, std::string> parseLine(const std::string& line);

		void cleanContent();
		std::vector<std::string> split(const std::string& str, char delimiter);
		std::string trim(const std::string& str);

	public:
		ConfigParser();
		ConfigParser(const std::string& configFilePath);
		ConfigParser(const ConfigParser& other);
		ConfigParser& operator=(const ConfigParser& other);
		~ConfigParser();

		void parseFile();

		const std::vector<ServerConfig>& getServers();

		class ErrorConfig : public std::exception
		{
			private:
				std::string _message;
			public:
				ErrorConfig(std::string message) throw()
				{
					_message = "CONFIG ERROR: " + message;
				}
				virtual const char* what() const throw()
				{
					return (_message.c_str());
				}
				virtual ~ErrorConfig() throw() {}
		};
};

#endif 

