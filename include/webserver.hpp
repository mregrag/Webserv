#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <map>
#include <string>
#include <stdexcept>
#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>

#include "Logger.hpp"
#include "EpollManager.hpp"
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "Client.hpp"
#include "ServerConfig.hpp"
#include "ConfigFile.hpp"
#include "ServerManager.hpp"



#define QUEUE_CONNEXION 5
#define MAX_EVENTS 100

template <typename T>
std::string toString(const T& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

#endif
