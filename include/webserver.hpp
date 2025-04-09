#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"
#include "EpollManager.hpp"
#include "ServerConfig.hpp"
#include "ServerInstance.hpp"
#include "ClientSession.hpp"

#include "ServerManager.hpp"


#define QUEUE_CONNEXION 5
#define MAX_EVENTS 100

#endif