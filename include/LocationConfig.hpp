/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 16:54:43 by mregrag           #+#    #+#             */
/*   Updated: 2025/02/25 23:17:41 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATIONCONFIG_HPP
#define LOCATIONCONFIG_HPP

#include <string>
#include <vector>


class LocationConfig
{
	private:
		std::string _path;
		std::string _root;
		std::string _index;
		std::vector<std::string> _allowedMethods;
		bool _autoindex;

	public:
		// Orthodox Canonical Form
		LocationConfig();
		LocationConfig(const LocationConfig& other);
		LocationConfig& operator=(const LocationConfig& other);
		~LocationConfig();

		// Getters
		const std::string& getPath() const;
		const std::string& getRoot() const;
		const std::string& getIndex() const;
		const std::vector<std::string>& getAllowedMethods() const;
		bool getAutoindex() const;

		// Setters
		void setPath(const std::string& path);
		void setRoot(const std::string& root);
		void setIndex(const std::string& index);
		void addAllowedMethod(const std::string& method);
		void setAutoindex(bool autoindex);
};
#endif // LOCATION_CONFIG_HPP

