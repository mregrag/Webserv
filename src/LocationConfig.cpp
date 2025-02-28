/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/04 17:26:11 by mregrag           #+#    #+#             */
/*   Updated: 2025/02/25 23:19:55 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/LocationConfig.hpp"
#include <iostream>
#include <algorithm>

// LocationConfig Implementation
LocationConfig::LocationConfig() : _autoindex(false) {}

LocationConfig::LocationConfig(const LocationConfig& other) :
    _path(other._path),
    _root(other._root),
    _index(other._index),
    _allowedMethods(other._allowedMethods),
    _autoindex(other._autoindex) {}

LocationConfig& LocationConfig::operator=(const LocationConfig& other) {
    if (this != &other) {
        _path = other._path;
        _root = other._root;
        _index = other._index;
        _allowedMethods = other._allowedMethods;
        _autoindex = other._autoindex;
    }
    return *this;
}

LocationConfig::~LocationConfig() {}

const std::string& LocationConfig::getPath() const {
    return _path;
}

const std::string& LocationConfig::getRoot() const {
    return _root;
}

const std::string& LocationConfig::getIndex() const {
    return _index;
}

const std::vector<std::string>& LocationConfig::getAllowedMethods() const {
    return _allowedMethods;
}

bool LocationConfig::getAutoindex() const {
    return _autoindex;
}

void LocationConfig::setPath(const std::string& path) {
    _path = path;
}

void LocationConfig::setRoot(const std::string& root) {
    _root = root;
}

void LocationConfig::setIndex(const std::string& index) {
    _index = index;
}

void LocationConfig::addAllowedMethod(const std::string& method) {
    _allowedMethods.push_back(method);
}

void LocationConfig::setAutoindex(bool autoindex) {
    _autoindex = autoindex;
}
