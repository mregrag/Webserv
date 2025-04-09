/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: zel-oirg <zel-oirg@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 17:29:36 by mregrag           #+#    #+#             */
/*   Updated: 2025/04/09 21:55:49 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ConfigParser.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>

ConfigParser::ConfigParser() : _configFile() 
{
}

ConfigParser::ConfigParser(const std::string& configFilePath) : _configFile(configFilePath)
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
		_configFile = other._configFile;
		_servers = other._servers;
	}
	return *this;
}

ConfigParser::~ConfigParser() 
{
}

const std::vector<ServerConfig>& ConfigParser::getServers()
{
	return _servers;
}

void ConfigParser::cleanContent() 
{
	size_t pos = 0;
	while ((pos = _content.find('#', pos)) != std::string::npos) 
	{
		size_t end = _content.find('\n', pos);
		_content.erase(pos, end != std::string::npos ? end - pos : std::string::npos);
	}

	// Trim leading whitespace
	pos = _content.find_first_not_of(" \t\r\n");
	if (pos != std::string::npos)
		_content.erase(0, pos);
	else 
	{
		_content.clear();
		return;
	}

	// Trim trailing whitespace
	pos = _content.find_last_not_of(" \t\r\n");
	if (pos != std::string::npos)
		_content.erase(pos + 1);

	// Normalize whitespace (collapse multiple spaces to single space)
	std::string cleaned;
	bool inSpace = false;
	for (size_t i = 0; i < _content.size(); ++i) 
	{
		if (_content[i] == '\n') 
		{
			cleaned += '\n';
			inSpace = false;
		}
		else if (std::isspace(_content[i])) 
		{
			if (!inSpace) 
			{
				cleaned += ' ';
				inSpace = true;
			}
		}
		else 
		{
			cleaned += _content[i];
			inSpace = false;
		}
	}
	_content = cleaned;
}

void ConfigParser::parseFile() 
{
	validateConfigFile();

	readAndPrepareContent();

	parseServerBlocks();

	if (_servers.empty()) 
		throw ErrorConfig("No valid server configurations found in file: " + _configFile.getPath());
}

void ConfigParser::validateConfigFile()
{
	const std::string& path = _configFile.getPath();

	switch (_configFile.getTypePath(path)) 
	{
		case 0: 
			throw ErrorConfig("Configuration file does not exist: " + path);
		case 2: 
			throw ErrorConfig("Path is a directory, not a file: " + path);
		case 3: 
			throw ErrorConfig("Path is a special file: " + path);
		case 1: 
			break;
		default: 
			throw ErrorConfig("Unknown file type: " + path);
	}
	if (!_configFile.checkFile(path, 0)) 
		throw ErrorConfig("No read permission for file: " + path);
}

void ConfigParser::readAndPrepareContent()
{
	const std::string& path = _configFile.getPath();

	if (!_configFile.readFile(path, _content)) 
		throw ErrorConfig("Failed to read file: " + path);

	if (_content.empty()) 
		throw ErrorConfig("File is empty: " + path);

	cleanContent();

	if (_content.empty()) 
		throw ErrorConfig("No valid configuration _content found after cleaning: " + path);
}

/**
 * @brief Parse all server blocks in the configuration _content
 * @param _content The prepared configuration _content
 */
void ConfigParser::parseServerBlocks()
{
	size_t pos = 0;
	bool foundServer = false;

	while (pos < _content.length()) 
	{
		// Find the next "server" keyword
		pos = findNextServerKeyword(pos);
		if (pos == std::string::npos)
			break;

		foundServer = true;
		size_t blockStart = findOpeningBrace(pos);
		if (blockStart == std::string::npos) 
			throw ErrorConfig("Missing '{' after 'server' keyword");

		// Extract server block with proper brace matching
		std::pair<std::string, size_t> blockInfo = extractBlock(blockStart);
		std::string serverBlock = blockInfo.first;

		try 
		{
			parseServerBlock(serverBlock);
		}
		catch (const ErrorConfig& e) 
		{
			throw ErrorConfig(std::string("Error in server block: ") + e.what());
		}

		// Move position past this server block
		pos = blockInfo.second;
	}

	// If "server" keyword was found but no valid servers were added
	if (foundServer && _servers.empty()) 
		throw ErrorConfig("No valid server configurations found despite server keywords");
}

/**
 * @brief Find the next occurrence of the server keyword
 * @param _content The configuration _content
 * @param startPos Position to start searching from
 * @return Position of server keyword or npos if not found
 */
size_t ConfigParser::findNextServerKeyword(size_t startPos)
{
	size_t pos = startPos;

	while (true) 
	{
		// Find the next occurrence of "server"
		pos = _content.find("server", pos);
		if (pos == std::string::npos)
			return std::string::npos;

		// Verify it's a standalone keyword (not part of another word)
		bool isValidKeyword = true;

		// Check character before (if not at beginning)
		if (pos > 0 && !isspace(_content[pos - 1]) && _content[pos - 1] != ';' && 
				_content[pos - 1] != '{' && _content[pos - 1] != '}') {
			isValidKeyword = false;
		}

		// Check character after (if not at end)
		if (pos + 6 < _content.length() && !isspace(_content[pos + 6]) && _content[pos + 6] != '{') 
			isValidKeyword = false;

		if (isValidKeyword)
			return pos;

		// Move past this occurrence
		pos += 6;
	}
}

/**
 * @brief Find the opening brace after a server keyword
 * @param _content The configuration _content
 * @param startPos Position of the server keyword
 * @return Position of opening brace or npos if not found
 */
size_t ConfigParser::findOpeningBrace(size_t startPos)
{
	size_t pos = startPos + 6; // Move past "server"

	// Skip whitespace
	while (pos < _content.length() && isspace(_content[pos]))
		++pos;

	// Check if we found an opening brace
	return (pos < _content.length() && _content[pos] == '{') ? pos : std::string::npos;
}

/**
 * @brief Extract a block of configuration with proper brace matching
 * @param _content The configuration _content
 * @param openBracePos Position of opening brace
 * @return Pair of block _content and position after block
 */
std::pair<std::string, size_t> ConfigParser::extractBlock(size_t openBracePos)
{
	int braceCount = 1;
	size_t closeBracePos = openBracePos + 1;

	while (braceCount > 0 && closeBracePos < _content.length()) 
	{
		if (_content[closeBracePos] == '{')
			++braceCount;
		else if (_content[closeBracePos] == '}')
			--braceCount;

		++closeBracePos;
	}

	if (braceCount != 0) 
		throw ErrorConfig("Unmatched '{' in block starting at position ");

	// Extract the block _content (excluding the outer braces)
	std::string block = _content.substr(openBracePos + 1, closeBracePos - openBracePos - 2);

	return std::make_pair(block, closeBracePos);
}

/**
 * @brief Parse a server block into a ServerConfig object
 * @param block Content of server block
 */
void ConfigParser::parseServerBlock(const std::string& block) 
{
	ServerConfig server;
	size_t pos = 0;

	while (pos < block.size()) 
	{
		// Skip whitespace
		while (pos < block.size() && std::isspace(block[pos]))
			++pos;
		if (pos >= block.size())
			break;

		// Find the end of directive (semicolon or opening brace)
		size_t endDirective = block.find_first_of(";\n{", pos);
		if (endDirective == std::string::npos)
			break;

		// Extract directive line
		std::string directiveLine = trim(block.substr(pos, endDirective - pos));
		if (directiveLine.empty()) 
		{
			pos = endDirective + 1;
			continue;
		}

		// Split into key and value
		size_t spacePos = directiveLine.find_first_of(" \t");
		if (spacePos == std::string::npos) 
		{
			pos = endDirective + 1;
			continue;
		}

		std::string key = trim(directiveLine.substr(0, spacePos));
		std::string value = trim(directiveLine.substr(spacePos + 1));

		// Handle location blocks specially
		if (key == "location") 
		{
			// Find location block
			size_t openBrace = block.find('{', endDirective);
			if (openBrace == std::string::npos)
				throw ErrorConfig("Missing '{' in location block");

			// Extract location block
			std::pair<std::string, size_t> locationInfo = extractBlock(openBrace);
			std::string locationBlock = locationInfo.first;

			// Parse location and add to server
			LocationConfig location = parseLocationBlock(locationBlock);
			server.addLocation(value, location);
			pos = locationInfo.second;
		} 
		else 
		{
			// Handle other server directives
			if (key == "host")
				server.setHost(value);
			else if (key == "listen")
				server.setPort(value);
			else if (key == "server_name")
				server.setServerName(value);
			else if (key == "client_max_body_size")
				server.setClientMaxBodySize(std::atoi(value.c_str()));
			else if (key == "error_page")
			{
				// Parse error page directive (format: error_page CODE PATH)
				std::vector<std::string> tokens = split(value, ' ');
				if (tokens.size() == 2)
					server.setErrorPage(std::atoi(tokens[0].c_str()), tokens[1]);
			}
			// Move past the directive
			pos = endDirective + 1;
		}
	}

	// Add the parsed server to the servers vector
	_servers.push_back(server);
}

/**
 * @brief Parse a location block into a LocationConfig object
 * @param block Content of location block
 * @return Parsed LocationConfig object
 */
LocationConfig ConfigParser::parseLocationBlock(const std::string& block) 
{
	LocationConfig location;
	std::istringstream iss(block);
	std::string line;

	while (std::getline(iss, line))
	{
		line = trim(line);
		if (line.empty() || line == "}")
			continue;

		std::pair<std::string, std::string> keyValue = parseLine(line);
		const std::string& key = keyValue.first;
		const std::string& value = keyValue.second;

		// Set location properties based on directive
		if (key == "root")
			location.setRoot(value);
		else if (key == "index")
			location.setIndex(value);
		else if (key == "autoindex")
			location.setAutoindex(value);
		else if (key == "allow_methods")
			location.setAllowedMethods(value);
		else if (key == "cgi_extension")
			location.setCgiExtension(value);
		else if (key == "cgi_path")
			location.setCgiPath(value);
	}
	return location;
}

/**
 * @brief Parse a single configuration line into key-value pair
 * @param line Line to parse
 * @return Pair of key and value
 */
std::pair<std::string, std::string> ConfigParser::parseLine(const std::string& line) 
{
	if (line == "}")
		return std::make_pair("}", "");

	size_t pos = line.find_first_of(" \t");
	if (pos == std::string::npos)
		throw ErrorConfig("Invalid config line: " + line);

	std::string key = trim(line.substr(0, pos));
	std::string value = trim(line.substr(pos + 1));

	// Remove trailing semicolon or brace if present
	if (!value.empty() && (value[value.size() - 1] == ';' || value[value.size() - 1] == '{'))
		value = value.substr(0, value.size() - 1);

	return std::make_pair(key, trim(value));
}

/**
 * @brief Split a string by a delimiter
 * @param str String to split
 * @param delimiter Character to split by
 * @return Vector of split strings
 */
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

/**
 * @brief Trim whitespace from beginning and end of string
 * @param str String to trim
 * @return Trimmed string
 */
std::string ConfigParser::trim(const std::string& str) 
{
	size_t first = str.find_first_not_of(" \t");
	if (first == std::string::npos)
		return "";

	size_t last = str.find_last_not_of(" \t");
	return str.substr(first, last - first + 1);
}

