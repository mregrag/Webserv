#ifndef SERVER_MANAGER_HPP
#define SERVER_MANAGER_HPP

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include "ServerConfig.hpp"
#include "EpollManager.hpp"
#include "Logger.hpp"
#include "../include/Client.hpp"

class ServerManager
{
	private:
		EpollManager _epollManager;
		std::vector<ServerConfig> _servers;
		std::map<int, ServerConfig*> _server_map;
		std::map<int, std::string> _client_requests;
		 std::map<int, Client> _clients;

	public:
		ServerManager();
		~ServerManager();

		void setupServers(const std::vector<ServerConfig>& servers);
		void run();
		void handleEvent(struct epoll_event& event);
		void acceptConnection(ServerConfig& server);
		void readRequest(int client_fd);
		void sendResponse(int client_fd);
		void closeConnection(int client_fd);

		class ErrorServer : public std::exception
		{
			private:
				std::string _message;
			public:
				ErrorServer(std::string message) throw()
				{
					_message = "SERVER ERROR: " + message;
				}
				virtual const char* what() const throw()
				{
					return (_message.c_str());
				}
				virtual ~ErrorServer() throw() {}
		};

	
};

#endif


