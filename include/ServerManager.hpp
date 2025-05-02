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
	public:
		ServerManager();
		~ServerManager();

		void setupServers(const std::vector<ServerConfig>& servers);

		void run();

	private:
		static const int MAX_EVENTS = 100; // Maximum epoll events per wait
		static const int TIMEOUT_CHECK_INTERVAL = 10; // Seconds between timeout checks
		static const int CONNECTION_TIMEOUT = 30; // Seconds for client inactivity

		std::vector<ServerConfig> _servers; // Server configurations
		std::map<int, ServerConfig*> _serverMap; // Maps server fds to ServerConfig
		std::map<int, Client*> _clients; // Maps client fds to Client objects
		EpollManager _epollManager; // Manages epoll instance

		// Configure a socket as non-blocking
		void configureSocket(int fd);

		// Register a server socket with epoll
		void registerServerSocket(int server_fd, ServerConfig* server);

		// Close and clean up all resources
		void cleanup();

		// Process a single epoll event
		void handleEvent(const struct epoll_event& event);

		// Accept a new client connection
		void acceptNewConnection(int server_fd);

		// Handle client read/write events
		void processClientEvent(int client_fd, uint32_t events);

		// Close a client connection
		void closeClientConnection(int client_fd);

		// Check for and close timed-out client connections
		void checkClientTimeouts();
		void handleReadEvent(Client* client);
		void handleWriteEvent(Client* client);
};

#endif


