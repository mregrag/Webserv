/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 16:54:43 by mregrag           #+#    #+#             */
/*   Updated: 2025/04/12 20:18:51 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include <vector>

class LocationConfig 
{
	public:
		LocationConfig();
		~LocationConfig();
		LocationConfig(const LocationConfig& other);
		LocationConfig& operator=(const LocationConfig& other);

		void setRoot(const std::string& root);
		void setIndex(const std::string& index);
		void setAutoindex(const std::string& autoindex);
		void setAllowedMethods(const std::string& methods);
		void setCgiExtension(const std::string& extension);
		void setCgiPath(const std::string& path);

		const std::string& getRoot() const;
		const std::string& getIndex() const;
		bool getAutoindex() const;
		const std::vector<std::string>& getAllowedMethods() const;
		const std::string& getCgiExtension() const;
		const std::string& getCgiPath() const;
		std::string resolvePath(const std::string& requestPath) const;

		void setPath(const std::string& path);
		const std::string& getPath() const;


		void print() const;
		std::vector<std::string> split(const std::string& str, char delimiter);
		std::string trim(const std::string& str);

	private:
		std::string _root;
		std::string _path;
		std::string _index;
		bool _autoindex;
		std::vector<std::string> _allowedMethods;
		std::string _cgiExtension;
		std::string _cgiPath;
};

#endif
