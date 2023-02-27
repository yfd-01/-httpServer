#include "server.h"
#include <cassert>

const int Server::MAX_FD = 65535;

void Server::running() {
    int timeout_interval = -1;

    while (1) {
        if (m_timeoutMS > 0) 
            timeout_interval = m_timer->getNextTick();

        int readyCnt = m_epoller->wait(timeout_interval);

        for (int i = 0; i < readyCnt; i++) {
            int fd = m_epoller->getFd(i);
            uint32_t events = m_epoller->getEvents(i);

            if (fd == m_listenFd) 
                handleListen();
            else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) 
                handleClose(&m_users[fd]);
            else if (events & EPOLLIN) 
                handleRead(&m_users[fd]);
            else if (events & EPOLLOUT) 
                handleWrite(&m_users[fd]);
            else {
                std::string msg = "unresolved events: " + std::to_string(events);
                Logger::Instance()->LOG_ERROR(msg);
            }
        }
    }
}

void Server::handleListen() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);

    do {
        int fd = accept(m_listenFd, (struct sockaddr*)&addr, &len);
        
        if (fd <= 0)
            return;
        else if (m_users.size() >= MAX_FD) {
            fulledReject(fd, "The connection was interrupted due to server overload");
            Logger::Instance()->LOG_WARNING("server busy");
            return;
        }

        // add client
        if (m_timeoutMS > 0) 
            m_timer->add(fd, m_timeoutMS, std::bind(&Server::handleClose, this, &m_users[fd]));

        // m_users[fd].init();
        m_epoller->addFd(fd, EPOLLIN | m_connEvents);

        std::string msg = "Client" + std::to_string(fd) + " conn in";
        Logger::Instance()->LOG_INFO(msg);

    } while (m_listenEvents & EPOLLET); // accept all if listen mode is ET
}

void Server::handleClose(HttpConn* conn) {
    assert(conn);

    conn->close();
    m_epoller->delFd(conn->getFd());

    std::string msg = "Client" + std::to_string(conn->getFd()) + " conn close";
    Logger::Instance()->LOG_INFO(msg);
}

void Server::handleRead(HttpConn* conn) {
    assert(conn);

    extendExpire(conn);
    m_threadPool->addTask(std::bind(&Server::_doRead, this, conn));
}

void doRead(HttpConn* conn) {
    
}

void Server::handleWrite(HttpConn* conn) {

}

void Server::initEventsMode(int choice) {
    m_listenEvents = EPOLLRDHUP;
    m_connEvents = EPOLLRDHUP | EPOLLONESHOT;   // EPOLLONESHOT - 保证一个socket连接在任一时刻只被一个线程处理

    switch(choice) {
        case 0:
            break;
        case 1:
            m_listenEvents |= EPOLLET;
            break;
        case 2:
            m_connEvents |= EPOLLET;
            break;
        case 3:
            m_listenEvents |= EPOLLET;
            m_connEvents |= EPOLLET;
            break;
        default:
            m_listenEvents |= EPOLLET;
            m_connEvents |= EPOLLET;
    }

    HttpConn::s_useET = (m_connEvents & EPOLLET);
}

void Server::fulledReject(int fd, const char* msg) {
    assert(fd > 0);

    if (send(fd, msg, strlen(msg), 0) < 0) {
        std::string msg = "the msg that indicated server is busy has been sended in error, fd: ";
        msg += std::to_string(fd);

        Logger::Instance()->LOG_ERROR(msg);
    }

    close(fd);
}

void Server::extendExpire(HttpConn* conn) {
    assert(conn->getFd() > 0);

    if (m_timeoutMS > 0)
        m_timer->adjust(conn->getFd(), m_timeoutMS);
}
