/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFile.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/28 00:23:50 by mregrag           #+#    #+#             */
/*   Updated: 2025/02/28 00:24:13 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_FILE_HPP
#define CONFIG_FILE_HPP

#include <string>
#include <sys/stat.h>
#include <fstream>
#include <sstream>

class ConfigFile
{
	private:
		std::string _path;
		size_t		_size;  // File size (if needed)

	public:
		// Default constructor
		ConfigFile();
		// Constructor with path parameter
		ConfigFile(std::string const path);
		// Copy constructor
		ConfigFile(const ConfigFile &other);
		// Assignment operator
		ConfigFile &operator=(const ConfigFile &other);
		// Destructor
		~ConfigFile();

		// Returns type of path: 0 = not exist, 1 = file, 2 = directory, 3 = other
		static int getTypePath(std::string const path);
		// Check file with mode (0 = read, 1 = write)
		static int checkFile(std::string const path, int mode);
		// Read file contents and return as string
		std::string readFile(std::string path);
		// Check if file exists and is readable; if not, try with the provided index suffix
		static int isFileExistAndReadable(std::string const path, std::string const index);

		// Getters for private members
		std::string getPath();
		int getSize();
};

#endif // CONFIG_FILE_HPP

