#include "../include/Epoll.hpp"
#include <iostream>

Epoll::Epoll() : _epollFd(-1)
{
	_epollFd = epoll_create(1);
	if (_epollFd == -1)
		throw std::runtime_error(std::string("Failed to create epoll"));
}

Epoll::~Epoll()
{
	if (_epollFd != -1)
	{
		close(_epollFd);
		_epollFd = -1;
	}
}

bool Epoll::add(int fd, uint32_t events)
{
	if (fd < 0)
		return false;

	epoll_event ev = {};
	ev.events = events;
	ev.data.fd = fd;

	return (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &ev) != -1);
}

bool Epoll::modify(int fd, uint32_t events)
{
	if (fd < 0)
		return false;

	epoll_event ev = {};
	ev.events = events;
	ev.data.fd = fd;

	return (epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &ev) != -1);
}

int Epoll::wait(int timeout) 
{
	return epoll_wait(_epollFd, _events, EVENTS, timeout);
}

bool Epoll::remove(int fd)
{
	if (fd < 0)
		return false;
	return (epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL) != -1);
}


int Epoll::getFd() const
{
	return _epollFd;
}

const epoll_event& Epoll::getEvent(int index) const
{
	if (index < 0 || index >= EVENTS)
		throw std::out_of_range("Event index out of range");
	return _events[index];
}
