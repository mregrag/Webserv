#include "../include/webserver.hpp"

EpollManager::EpollManager()
{
    _epollFd = epoll_create1(0);
    
    int flags = fcntl(_epollFd, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error("fcntl get failed");

    if (fcntl(_epollFd, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl set non-blocking failed");

    if (_epollFd == -1)
        throw (std::runtime_error("epoll_create1()"));
}

EpollManager::~EpollManager()
{
    close(_epollFd);
}

void    EpollManager::addFd(int fd, uint32_t events)
{
    struct epoll_event  event;
    event.data.fd = fd;
    event.events = events;
    if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &event) == -1)
        throw std::runtime_error("Failed to add fd to epoll");
}

void    EpollManager::modifyFd(int fd, uint32_t events)
{
    struct epoll_event  event;
    event.data.fd = fd;
    event.events = events;
    if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &event) == -1)
        throw std::runtime_error("Failed to modify fd in epoll");
}

void    EpollManager::removeFd(int fd)
{
    if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL) == -1)
        throw std::runtime_error("Failed to remove fd from epoll");
}

int EpollManager::wait(std::vector<struct epoll_event>& events, int timeout)
{
    events.resize(MAX_EVENTS);
    int numEvent = epoll_wait(_epollFd, events.data(), MAX_EVENTS, timeout);
    if (numEvent == -1)
        throw std::runtime_error("epoll_wait failed");
    return numEvent;
}