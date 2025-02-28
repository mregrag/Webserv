/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 17:29:36 by mregrag           #+#    #+#             */
/*   Updated: 2025/02/26 00:06:38 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ConfigParser.hpp"
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <iostream>

// ConfigParser Implementation
ConfigParser::ConfigParser()
{
}

ConfigParser::ConfigParser(const std::string& configFilePath) : _configFilePath(configFilePath)
{
}

ConfigParser::ConfigParser(const ConfigParser& rhs) : _servers(rhs._servers), _configFilePath(rhs._configFilePath)
{
}

ConfigParser& ConfigParser::operator=(const ConfigParser& rhs)
{
	if (this != &rhs)
	{
		_servers = rhs._servers;
		_configFilePath = rhs._configFilePath;
	}
	return (*this);
}

ConfigParser::~ConfigParser() 
{
}

bool ConfigParser::parse()
{
	try
	{
		parseFile();
		return true;
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error parsing config file: " << e.what() << std::endl;
		return (false);
	}
}

const std::vector<ServerConfig>& ConfigParser::getServers() const
{
	return (_servers);
}

void ConfigParser::parseFile()
{
	std::ifstream file(_configFilePath.c_str());
	if (!file.is_open())
		throw std::runtime_error("Cannot open config file");

	std::string line;
	std::string serverBlock;
	bool inServerBlock = false;
	int braceCount = 0;

	while (std::getline(file, line))
	{
		line = trim(line);

		// Skip comments and empty lines
		if (line.empty() || line[0] == '#')
			continue;

		if (line.find("server {") != std::string::npos)
		{
			inServerBlock = true;
			braceCount = 1;
			serverBlock = line + "\n";
		}
		else if (inServerBlock)
		{
			serverBlock += line + "\n";

			if (line.find("{") != std::string::npos)
				braceCount++;
			if (line.find("}") != std::string::npos)
				braceCount--;

			if (braceCount == 0) 
			{
				inServerBlock = false;
				parseServerBlock(serverBlock);
				serverBlock.clear();
			}
		}
	}

	file.close();
}

void ConfigParser::parseServerBlock(const std::string& block)
{
	ServerConfig config;
	std::istringstream iss(block);
	std::string line;
	std::string locationBlock;
	bool inLocationBlock = false;
	int braceCount = 0;

	while (std::getline(iss, line))
	{
		line = trim(line);

		// Skip comments and empty lines
		if (line.empty() || line[0] == '#')
			continue;

		if (line.find("location ") != std::string::npos && line.find("{") != std::string::npos)
		{
			inLocationBlock = true;
			braceCount = 1;
			locationBlock = line + "\n";
		}
		else if (inLocationBlock)
		{
			locationBlock += line + "\n";

			if (line.find("{") != std::string::npos)
				braceCount++;
			if (line.find("}") != std::string::npos)
				braceCount--;

			if (braceCount == 0)
			{
				inLocationBlock = false;
				LocationConfig locConfig = parseLocationBlock(locationBlock);
				config.addLocation(locConfig);
				locationBlock.clear();
			}
		}
		else if (line != "server {" && line != "}")
		{
			std::pair<std::string, std::string> directive = parseLine(line);

			if (directive.first == "listen")
				config.setPort(atoi(directive.second.c_str()));
			else if (directive.first == "host")
				config.setHost(directive.second);
			else if (directive.first == "server_name")
				config.setServerName(directive.second);
			else if (directive.first == "client_max_body_size")
				config.setClientMaxBodySize(atoi(directive.second.c_str()));
			else if (directive.first == "error_page")
			{
				std::vector<std::string> parts = split(directive.second, ' ');
				if (parts.size() == 2)
					config.addErrorPage(atoi(parts[0].c_str()), parts[1]);
			}
		}
	}

	_servers.push_back(config);
}

LocationConfig ConfigParser::parseLocationBlock(const std::string& block)
{
	LocationConfig config;
	std::istringstream iss(block);
	std::string line;

	// Extract location path
	std::getline(iss, line);
	line = trim(line);
	size_t pathStart = line.find("location ") + 9;
	size_t pathEnd = line.find(" {");
	if (pathEnd == std::string::npos)
		pathEnd = line.find("{");
	std::string path = line.substr(pathStart, pathEnd - pathStart);
	config.setPath(trim(path));

	// Parse location directives
	while (std::getline(iss, line))
	{
		line = trim(line);

		// Skip comments, empty lines, and closing brace
		if (line.empty() || line[0] == '#' || line == "}")
			continue;

		std::pair<std::string, std::string> directive = parseLine(line);

		if (directive.first == "root")
			config.setRoot(directive.second);
		else if (directive.first == "index")
			config.setIndex(directive.second);
		else if (directive.first == "allow_methods") 
		{
			std::vector<std::string> methods = split(directive.second, ' ');
			for (size_t i = 0; i < methods.size(); ++i)
				config.addAllowedMethod(methods[i]);
		}
		else if (directive.first == "autoindex") 
			config.setAutoindex(directive.second == "on");
	}

	return (config);
}

std::pair<std::string, std::string> ConfigParser::parseLine(const std::string& line)
{
	size_t pos = line.find(";");
	if (pos == std::string::npos)
		return std::make_pair("", "");

	std::string directiveLine = line.substr(0, pos);
	pos = directiveLine.find(" ");
	if (pos == std::string::npos)
		return std::make_pair(directiveLine, "");

	std::string directive = directiveLine.substr(0, pos);
	std::string value = directiveLine.substr(pos + 1);

	return std::make_pair(trim(directive), trim(value));
}

std::string ConfigParser::trim(const std::string& str)
{
	size_t first = str.find_first_not_of(" \t");
	if (first == std::string::npos)
		return ("");
	size_t last = str.find_last_not_of(" \t");
	return (str.substr(first, last - first + 1));
}

std::vector<std::string> ConfigParser::split(const std::string& str, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(str);

	while (std::getline(tokenStream, token, delimiter))
	{
		token = trim(token);
		if (!token.empty())
			tokens.push_back(token);
	}

	return tokens;
}
