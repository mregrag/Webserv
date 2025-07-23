#ifndef EPOLLMANAGER_HPP
#define EPOLLMANAGER_HPP

#include <sys/epoll.h>
#include <vector>
#include <stdexcept>
#include <unistd.h>

#define EVENTS 1024

class Epoll 
{
public:
	Epoll();
	~Epoll();

	Epoll(const Epoll&);
	Epoll& operator=(const Epoll&);

	bool add(int fd, uint32_t events);
	bool modify(int fd, uint32_t events);
	bool remove(int fd);
	int wait(int timeout = -1);
	const epoll_event& getEvent(int index) const;
	int getFd() const;

private:
	int _epollFd;
	epoll_event _events[EVENTS];
};

#endif
