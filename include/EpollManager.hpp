#pragma once
#include <sys/epoll.h>
#include <vector>
#include <string>

class EpollManager 
{
public:
	explicit EpollManager(size_t maxEvents);
	~EpollManager();

	// Prevent copying
	EpollManager(const EpollManager&);
	EpollManager& operator=(const EpollManager&);

	bool add(int fd, uint32_t events);
	bool modify(int fd, uint32_t events);
	bool remove(int fd);
	int wait(int timeout = -1);
	const epoll_event& getEvent(size_t index) const;
	int getFd() const { return _epollFd; }

private:
	int _epollFd;
	std::vector<epoll_event> _events;
};
