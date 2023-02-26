#ifndef _EPOLLER_H
#define _EPOLLER_H

#include <sys/epoll.h>
#include <vector>
#include <unistd.h>

class Epoller {
public:
    Epoller(int maxEvent = 1024);
    ~Epoller();

    bool addFd(int fd, uint32_t events);
    bool modFd(int fd, uint32_t events);
    bool delFd(int fd);

    int wait(int timeout);

private:
    int m_epoll_fd;
    std::vector<struct epoll_event> m_events;
};

#endif  // _EPOLLER_H