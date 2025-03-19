/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 17:26:11 by mregrag           #+#    #+#             */
/*   Updated: 2025/03/18 20:20:00 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/LocationConfig.hpp"
#include <iostream>

LocationConfig::LocationConfig() : _root(""), _index(""), _autoindex(false), _cgiExtension(""), _cgiPath("") {}

LocationConfig::~LocationConfig() {}

LocationConfig::LocationConfig(const LocationConfig& other) {
	*this = other;
}

LocationConfig& LocationConfig::operator=(const LocationConfig& other)
{
	if (this != &other) {
		_root = other._root;
		_index = other._index;
		_autoindex = other._autoindex;
		_allowedMethods = other._allowedMethods;
		_cgiExtension = other._cgiExtension;
		_cgiPath = other._cgiPath;
	}
	return *this;
}

void LocationConfig::setRoot(const std::string& root)
{
	_root = root; 
}

void LocationConfig::setIndex(const std::string& index)
{
	_index = index;
}
void LocationConfig::setAutoindex(bool autoindex) 
{
	_autoindex = autoindex;
}
void LocationConfig::setAllowedMethods(const std::vector<std::string>& methods)
{ 
	_allowedMethods = methods;
}
void LocationConfig::setCgiExtension(const std::string& extension)
{ 
	_cgiExtension = extension;
}
void LocationConfig::setCgiPath(const std::string& path)
{
	_cgiPath = path;
}

const std::string& LocationConfig::getRoot() const 
{
	return _root;
}
const std::string& LocationConfig::getIndex() const
{
	return _index;
}
bool LocationConfig::getAutoindex() const
{
	return _autoindex;
}
const std::vector<std::string>& LocationConfig::getAllowedMethods() const
{
	return _allowedMethods;
}
const std::string& LocationConfig::getCgiExtension() const
{
	return _cgiExtension; 
}
const std::string& LocationConfig::getCgiPath() const
{
	return _cgiPath;
}

void LocationConfig::print() const
{
	std::cout << "  Location Config:\n";
	std::cout << "    Root: " << _root << "\n";
	std::cout << "    Index: " << _index << "\n";
	std::cout << "    Autoindex: " << (_autoindex ? "on" : "off") << "\n";
	std::cout << "    Allowed Methods: ";
	for (size_t i = 0; i < _allowedMethods.size(); ++i) {
		std::cout << _allowedMethods[i] << " ";
	}
	std::cout << "\n";
	std::cout << "    CGI Extension: " << _cgiExtension << "\n";
	std::cout << "    CGI Path: " << _cgiPath << "\n";
}
