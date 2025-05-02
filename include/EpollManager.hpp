#ifndef EPOLLMANAGER_HPP
#define EPOLLMANAGER_HPP

#include <vector>
#include <sys/epoll.h>
#include <stdexcept>
#include <unistd.h>
#include <cstring>

class EpollManager
{
	private:
		int _epollFd;

	public:
		EpollManager();
		~EpollManager();

		// No copy constructor or assignment operator
		EpollManager(const EpollManager& other);
		EpollManager& operator=(const EpollManager& other);

		void addFd(int fd, uint32_t events);
		void modifyFd(int fd, uint32_t events);
		void removeFd(int fd);
		int wait(std::vector<struct epoll_event>& events, int timeout);
};


#endif
