#include "epoller.h"

Epoller::Epoller(int maxEvent) {
    m_epoll_fd = epoll_create(1);
    m_events.reserve(maxEvent);

    assert(m_epoll_fd >=0 && m_events.size() > 0);
}

Epoller::~Epoller() {
    close(m_epoll_fd);
}


bool Epoller::addFd(int fd, uint32_t events) {
    assert(fd >= 0);

    epoll_event ev = { 0 };
    ev.data.fd = fd;
    ev.events = events;

    return epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, fd, &ev) == 0;
}

bool Epoller::modFd(int fd, uint32_t events) {
    assert(fd >= 0);

    epoll_event ev = { 0 };
    ev.data.fd = fd;
    ev.events = events;

    return epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, fd, &ev) == 0;
}

bool Epoller::delFd(int fd) {
    assert(fd >= 0);

    epoll_event ev = {};
    return epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, &ev);
}

int Epoller::wait(int timeout) {
    return epoll_wait(m_epoll_fd, &m_events[0], static_cast<int>(m_events.size()), timeout);
}


int Epoller::getFd(int i) const {
    assert(i >= 0 && i < m_events.size());

    return m_events[i].data.fd;
}

uint32_t Epoller::getEvents(int i) const {
    assert(i >= 0 && i < m_events.size());

    return m_events[i].events;
}
