/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zel-oirg <zel-oirg@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 17:29:36 by mregrag           #+#    #+#             */
/*   Updated: 2025/03/19 05:07:37 by zel-oirg         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ConfigParser.hpp"

ConfigParser::ConfigParser() : _configFilePath("")
{
}

ConfigParser::ConfigParser(const std::string& configFilePath) : _configFilePath(configFilePath)
{
}

ConfigParser::ConfigParser(const ConfigParser& other)
{
	*this = other;
}

ConfigParser& ConfigParser::operator=(const ConfigParser& other)
{
	if (this != &other)
	{
		_servers = other._servers;
		_configFilePath = other._configFilePath;
	}
	return *this;
}

ConfigParser::~ConfigParser()
{
}

bool ConfigParser::parse() 
{
	if (_configFilePath.empty())
		return false;

	parseFile();
	return true;
}

const std::vector<ServerConfig>& ConfigParser::getServers()
{
	return _servers;
}

void ConfigParser::print() const 
{
	for (size_t i = 0; i < _servers.size(); ++i)
	{
		std::cout << "Server " << i + 1 << ":\n";
		// _servers[i].print();
	}
}

void ConfigParser::parseFile()
{
	std::ifstream file(_configFilePath.c_str());
	if (!file.is_open())
		throw std::runtime_error("Failed to open configuration file: " + _configFilePath);

	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string content = buffer.str();

	size_t pos = 0;
	while ((pos = content.find("server", pos)) != std::string::npos)
	{
		size_t blockStart = content.find("{", pos);
		if (blockStart == std::string::npos)
			throw std::runtime_error("Invalid server block: missing '{'");

		int braceCount = 1;
		size_t blockEnd = blockStart + 1;
		while (braceCount > 0 && blockEnd < content.size())
		{
			if (content[blockEnd] == '{')
				braceCount++;
			else if (content[blockEnd] == '}') 
				braceCount--;
			blockEnd++;
		}

		if (braceCount != 0)
			throw std::runtime_error("Invalid server block: missing '}'");

		std::string block = content.substr(blockStart + 1, blockEnd - blockStart - 2);
		parseServerBlock(block);
		pos = blockEnd;
	}
}

// Fixed parseServerBlock method to correctly handle multiple location blocks
void ConfigParser::parseServerBlock(const std::string& block)
{
	ServerConfig server;
	std::string remainingBlock = block;
	size_t pos = 0;

	while (pos < remainingBlock.size())
	{
		// Skip whitespace and comments
		while (pos < remainingBlock.size() && (isspace(remainingBlock[pos]) || remainingBlock[pos] == '#'))
		{
			if (remainingBlock[pos] == '#')
			{
				// Skip to end of line
				size_t endLine = remainingBlock.find('\n', pos);
				if (endLine == std::string::npos)
					pos = remainingBlock.size();
				else
					pos = endLine + 1;
			}
			else
				pos++;
		}

		if (pos >= remainingBlock.size())
			break;

		// Find the end of the directive
		size_t endDirective = remainingBlock.find_first_of(";\n{", pos);
		if (endDirective == std::string::npos)
			break;

		// Extract the directive line
		std::string directiveLine = trim(remainingBlock.substr(pos, endDirective - pos));
		if (directiveLine.empty())
		{
			pos = endDirective + 1;
			continue;
		}

		// Parse key and value from the directive
		size_t spacePos = directiveLine.find_first_of(" \t");
		if (spacePos == std::string::npos)
		{
			pos = endDirective + 1;
			continue;
		}

		std::string key = trim(directiveLine.substr(0, spacePos));
		std::string value = trim(directiveLine.substr(spacePos + 1));

		// Check if this is a location directive
		if (key == "location")
		{
			// Find the opening brace for the location block
			size_t openBrace = remainingBlock.find('{', endDirective);
			if (openBrace == std::string::npos)
				throw std::runtime_error("Invalid location block: missing '{'");

			// Find the matching closing brace
			int braceCount = 1;
			size_t closeBrace = openBrace + 1;
			while (braceCount > 0 && closeBrace < remainingBlock.size())
			{
				if (remainingBlock[closeBrace] == '{')
					braceCount++;
				else if (remainingBlock[closeBrace] == '}')
					braceCount--;
				closeBrace++;
			}

			if (braceCount != 0)
				throw std::runtime_error("Invalid location block: missing '}'");

			// Extract and parse the location block
			std::string locationBlock = remainingBlock.substr(openBrace + 1, closeBrace - openBrace - 2);
			LocationConfig location = parseLocationBlock(locationBlock);

			// Add the location to the server config
			server.addLocation(value, location);

			// Move past this location block
			pos = closeBrace;
		}
		else
		{
			// Handle normal directives
			if (key == "host")
				server.setHost(value);
			else if (key == "listen")
				server.setPort(value);
			else if (key == "server_name")
				server.setServerName(value);
			else if (key == "client_max_body_size")
				server.setClientMaxBodySize(atoi(value.c_str()));
			else if (key == "error_page")
			{
				std::vector<std::string> tokens = split(value, ' ');
				if (tokens.size() == 2)
					server.setErrorPage(atoi(tokens[0].c_str()), tokens[1]);
			}

			// Move past this directive
			pos = endDirective + 1;
		}
	}

	_servers.push_back(server);
}



// Improved parseLocationBlock to ensure all config values are correctly parsed
LocationConfig ConfigParser::parseLocationBlock(const std::string& block)
{
	LocationConfig location;
	std::istringstream iss(block);
	std::string line;

	while (std::getline(iss, line))
	{
		line = trim(line);

		// Skip empty lines, comments, and closing braces
		if (line.empty() || line[0] == '#' || line == "}")
			continue;

		// Parse key-value pair
		std::pair<std::string, std::string> keyValue = parseLine(line);
		const std::string& key = keyValue.first;
		const std::string& value = keyValue.second;

		// Set location properties
		if (key == "root")
			location.setRoot(value);
		else if (key == "index")
			location.setIndex(value);
		else if (key == "autoindex")
			location.setAutoindex(value == "on");
		else if (key == "allow_methods")
		{
			std::vector<std::string> methods = split(value, ' ');
			location.setAllowedMethods(methods);
		}
		else if (key == "cgi_extension")
			location.setCgiExtension(value);
		else if (key == "cgi_path")
			location.setCgiPath(value);
	}

	return location;
}


std::pair<std::string, std::string> ConfigParser::parseLine(const std::string& line) 
{
	// Remove comments from the line
	std::string cleanedLine = line;
	size_t commentPos = cleanedLine.find('#');
	if (commentPos != std::string::npos) 
		cleanedLine = cleanedLine.substr(0, commentPos);

	// Trim the cleaned line
	cleanedLine = trim(cleanedLine);

	// Skip lines that contain only a closing brace '}'
	if (cleanedLine == "}") 
		return std::make_pair("}", "");

	// Split the line into key and value
	size_t pos = cleanedLine.find_first_of(" \t");
	if (pos == std::string::npos) 
		throw std::runtime_error("Invalid configuration line: " + line);

	std::string key = trim(cleanedLine.substr(0, pos));
	std::string value = trim(cleanedLine.substr(pos + 1));

	// Remove trailing semicolon or opening brace from value
	if (!value.empty()) 
		if (value[value.size() - 1] == ';' || value[value.size() - 1] == '{') 
			value = value.substr(0, value.size() - 1);

	return std::make_pair(key, value);
}

std::string ConfigParser::trim(const std::string& str) 
{
	size_t first = str.find_first_not_of(" \t");
	size_t last = str.find_last_not_of(" \t");
	if (first == std::string::npos || last == std::string::npos) 
		return "";
	return str.substr(first, last - first + 1);
}

std::vector<std::string> ConfigParser::split(const std::string& str, char delimiter) 
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream iss(str);
	while (std::getline(iss, token, delimiter)) 
	{
		token = trim(token);
		if (!token.empty()) 
			tokens.push_back(token);
	}
	return tokens;
}
