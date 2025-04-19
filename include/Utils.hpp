/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/18 17:22:42 by mregrag           #+#    #+#             */
/*   Updated: 2025/04/18 22:42:56 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <map>
#include <sys/stat.h>
#include <dirent.h>
#include <sstream>
#include <vector>
#include <algorithm>


namespace Utils 
{
	template <typename T>
	std::string toString(const T& value)
	{
		std::ostringstream oss;
		oss << value;
		return oss.str();
	}	

	std::string getMimeType(const std::string &path);
	bool isDirectory(const std::string &path);
	std::string generateAutoIndex(const std::string &path, const std::string &requestUri);
	std::string listDirectory(const std::string& dirPath, const std::string& root, const std::string& requestUri);
	bool isPathWithinRoot(const std::string& root, const std::string& path);
	bool isDirectory(const std::string& path);
	std::string trim(const std::string& str);
}

#endif

