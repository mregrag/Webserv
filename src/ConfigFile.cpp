/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFile.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 00:24:59 by mregrag           #+#    #+#             */
/*   Updated: 2025/03/17 22:47:57 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/ConfigFile.hpp"

ConfigFile::ConfigFile() : _path(""), _size(0)
{
}

// Constructor with path parameter
ConfigFile::ConfigFile(std::string const path) : _path(path), _size(0)
{
	struct stat st;
	if (stat(path.c_str(), &st) == 0)
		_size = st.st_size;
}

// Copy constructor
ConfigFile::ConfigFile(const ConfigFile &rhs) : _path(rhs._path), _size(rhs._size)
{
}

// Assignment operator
ConfigFile &ConfigFile::operator=(const ConfigFile &rhs)
{
	if (this != &rhs)
	{
		_path = rhs._path;
		_size = rhs._size;
	}
	return *this;
}

// Destructor
ConfigFile::~ConfigFile()
{
	// Nothing to free for now
}

// Returns type of path: 0 = does not exist, 1 = regular file, 2 = directory, 3 = rhs type
int ConfigFile::getTypePath(std::string const path)
{
	struct stat st;
	if (stat(path.c_str(), &st) != 0) 
		return 0; // Does not exist
	if (S_ISREG(st.st_mode))
		return 1; // Regular file
	if (S_ISDIR(st.st_mode))
		return 2; // Directory
	return 3; // Other type (e.g., device, pipe, etc.)
}

// Check file with mode (0 = check for readability, 1 = check for writability)
int ConfigFile::checkFile(std::string const path, int mode)
{
	if (mode == 0) {
		std::ifstream infile(path.c_str());
		return (infile.good()) ? 1 : 0;
	} else if (mode == 1) {
		std::ofstream outfile(path.c_str(), std::ios::app);
		return (outfile.good()) ? 1 : 0;
	}
	return 0;
}

// Read file contents into a string
std::string ConfigFile::readFile(std::string path) {
	std::ifstream infile(path.c_str());
	if (!infile)
		return "";
	std::stringstream buffer;
	buffer << infile.rdbuf();
	return buffer.str();
}

// Check if file exists and is readable. If not, try appending the index string.
int ConfigFile::isFileExistAndReadable(std::string const path, std::string const index) {
	std::ifstream infile(path.c_str());
	if (infile.good())
		return 1;
	std::ifstream infile_index((path + index).c_str());
	return (infile_index.good()) ? 1 : 0;
}

// Getter for _path
std::string ConfigFile::getPath()
{
	return _path;
}

// Getter for _size
int ConfigFile::getSize()
{
	return static_cast<int>(_size);
}

