#ifndef _SERVER_H
#define _SERVER_H

#include <memory>
#include <unordered_map>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>

#include "epoller.h"
#include "../pool/threadPool.h"
#include "../timer/heapTimer.h"
#include "../http/httpConn.h"
#include "../logger/logger.h"

class Server {
public:
    Server();
    ~Server();

public:
    void running();

private:
    uint32_t m_listenEvents;
    uint32_t m_connEvents;

    int m_timeoutMS;
    int m_listenFd;

    static const int MAX_FD;

    std::unique_ptr<HeapTimer> m_timer;
    std::unique_ptr<Epoller> m_epoller;
    std::unique_ptr<ThreadPool> m_threadPool;

    std::unordered_map<int, HttpConn> m_users;

private:
    void initEventsMode(int choice);

    void handleListen();
    void handleClose(HttpConn* conn);
    void handleRead(HttpConn* conn);
    void handleWrite(HttpConn* conn);

    void fulledReject(int fd, const char* msg);
    void extendExpire(HttpConn* conn);

    void _doRead(HttpConn* conn);
    void _doWrite(HttpConn* conn);
};

#endif  // _SERVER_H