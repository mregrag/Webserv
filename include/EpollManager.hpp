#ifndef EPOLL_HPP
#define EPOLL_HPP

#include "webserver.hpp"

class EpollManager
{
public:
    EpollManager();
    ~EpollManager();

    void addFd(int fd, uint32_t events);   // Register a socket for monitoring
    void modifyFd(int fd, uint32_t events); // Change monitored events
    void removeFd(int fd);                 // Stop monitoring an FD
    int wait(std::vector<struct epoll_event>& events, int timeout); // Wait for events

private:
    int _epollFd; // File descriptor for the epoll instance
};


#endif