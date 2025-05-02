/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EpollManager.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mregrag <mregrag@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/10 15:05:56 by mregrag           #+#    #+#             */
/*   Updated: 2025/04/30 18:54:13 by mregrag          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserver.hpp"

EpollManager::EpollManager()
{
    _epollFd = epoll_create1(0);
    if (_epollFd == -1)
        throw std::runtime_error("Failed to create epoll instance: " + std::string(strerror(errno)));
    
    LOG_INFO("Epoll instance created with fd: " + Utils::toString(_epollFd));
}

EpollManager::~EpollManager()
{
    if (_epollFd != -1)
    {
        close(_epollFd);
        LOG_INFO("Epoll instance closed");
    }
}

EpollManager::EpollManager(const EpollManager& other)
{
    (void)other;
    throw std::runtime_error("EpollManager cannot be copied");
}

EpollManager& EpollManager::operator=(const EpollManager& other)
{
    (void)other;
    throw std::runtime_error("EpollManager cannot be assigned");
    return *this;
}

void EpollManager::addFd(int fd, uint32_t events)
{
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = events;
    event.data.fd = fd;

    if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &event) == -1)
        throw std::runtime_error("Failed to add fd to epoll: " + std::string(strerror(errno)));
    
    LOG_DEBUG("Added fd " + Utils::toString(fd) + " to epoll with events " + Utils::toString(events));
}

void EpollManager::modifyFd(int fd, uint32_t events)
{
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = events;
    event.data.fd = fd;

    if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &event) == -1)
        throw std::runtime_error("Failed to modify fd in epoll: " + std::string(strerror(errno)));
    
    LOG_DEBUG("Modified fd " + Utils::toString(fd) + " in epoll with events " + Utils::toString(events));
}

void EpollManager::removeFd(int fd)
{
    if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL) == -1)
        throw std::runtime_error("Failed to remove fd from epoll: " + std::string(strerror(errno)));
    
    LOG_DEBUG("Removed fd " + Utils::toString(fd) + " from epoll");
}

int EpollManager::wait(std::vector<struct epoll_event>& events, int timeout)
{
    int num_events = epoll_wait(_epollFd, &events[0], events.size(), timeout);
    
    if (num_events == -1)
    {
        // Ignore EINTR (interrupted system call)
        if (errno == EINTR)
            return 0;
            
        throw std::runtime_error("epoll_wait failed: " + std::string(strerror(errno)));
    }
    
    return num_events;
}
