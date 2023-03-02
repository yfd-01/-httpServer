#include "server.h"
#include <asm-generic/socket.h>
#include <cstdlib>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>

const int Server::MAX_FD = 65535;

Server::Server(
    BaseConfig* baseConfig, SQLConfig* sqlConfig, LoggerConfig* loggerConfig,
    int threadNums, int sqlConnNums, int loggerQueSize
) {
    char* srcDir = getcwd(nullptr, 256);

    HttpConn::s_usersCount = 0;
    HttpConn::s_srcDir = std::string(srcDir) + "/static/";  // 静态资源路径

    m_port = baseConfig->_port;
    m_timeoutMS = baseConfig->_timeoutMS;
    initEventsMode(baseConfig->_modeChoice);    // io多路复用类型

    // 日志模块初始化
    Logger::Instance()->init(loggerConfig->_level, loggerConfig->_device, loggerConfig->_path, loggerConfig->_suffix, loggerQueSize);

    // 数据库连接池模块初始化
    SqlConnPool::Instance()->init(sqlConnNums, sqlConfig);

    // 线程池模块初始化
    m_threadPool = std::make_unique<ThreadPool>(threadNums);

    // epoller && 时间最小堆 初始化
    m_epoller = std::make_unique<Epoller>();
    m_timer = std::make_unique<HeapTimer>();

    // 服务器端口初始化
    if (!Initialize(baseConfig->_lingerUsing)) {
        close(m_listenFd);
        Logger::Instance()->LOG_ERROR("服务器启动失败");
        exit(-1);
    }
}

Server::~Server() {

}

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
        else if (HttpConn::s_usersCount >= MAX_FD) {
            fulledReject(fd, "The connection was interrupted due to server overload");
            Logger::Instance()->LOG_WARNING("server busy");
            return;
        }

        // add client
        if (m_timeoutMS > 0) 
            m_timer->add(fd, m_timeoutMS, std::bind(&Server::handleClose, this, &m_users[fd]));

        if (m_users.count(fd) == 0) 
            m_users.emplace(fd, new HttpConn());
            
        m_users[fd].init(fd, addr);

        m_epoller->addFd(fd, EPOLLIN | m_connEvents);

        setNonBlocking(fd);
        std::string msg = "Client" + std::to_string(fd) + " conn in";
        Logger::Instance()->LOG_INFO(msg);

    } while (m_listenEvents & EPOLLET); // accept all if listen mode is ET
}

void Server::handleClose(HttpConn* conn) {
    assert(conn);

    conn->doClose();
    m_epoller->delFd(conn->getFd());

    std::string msg = "Client" + std::to_string(conn->getFd()) + " conn close";
    Logger::Instance()->LOG_INFO(msg);
}

void Server::handleRead(HttpConn* conn) {
    assert(conn);

    extendExpire(conn);
    m_threadPool->addTask(std::bind(&Server::_doRead, this, conn));
}

void Server::_doRead(HttpConn* conn) {
    assert(conn);

    int ret = -1;
    int readErrno = 0;
    
    ret = conn->read(&readErrno);
    if (ret < 0 && readErrno != EAGAIN) {
        handleClose(conn);
        return;
    }

    _doProcess(conn);
}

void Server::handleWrite(HttpConn* conn) {
    assert(conn);

    extendExpire(conn);
    m_threadPool->addTask(std::bind(&Server::_doWrite, this, conn));
}

void Server::_doWrite(HttpConn* conn) {
    assert(conn);

    int ret = -1;
    int writeErrno = 0;

    ret = conn->write(&writeErrno);

    if (conn->bytesToSend() == 0) { // has send all
        if (conn->isKeepAlive()) {
            _doProcess(conn);
            return;
        }
    }else if (ret < 0) {
        if (writeErrno == EAGAIN) { // try once
            m_epoller->modFd(conn->getFd(), m_connEvents | EPOLLOUT);
            return;
        }
    }

    handleClose(conn);
}

void Server::_doProcess(HttpConn* conn) {
    if (conn->process())
        m_epoller->modFd(conn->getFd(), m_connEvents | EPOLLOUT);
    else
        m_epoller->modFd(conn->getFd(), m_connEvents | EPOLLIN);
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

bool Server::Initialize(bool lingerUsing) {
    if (m_port < 1024 || m_port > 65535) {
        Logger::Instance()->LOG_ERROR("非合理端口");
        return false;
    }

    m_listenFd = socket(AF_INET, SOCK_STREAM, 0);

    if (m_listenFd < 0) {
        Logger::Instance()->LOG_ERROR("Create socket error");
        return false;
    }

    struct linger lingerOpt = { 0 };
    if (lingerUsing) {
        lingerOpt.l_onoff = 1;
        lingerOpt.l_linger = 3;
    }

    int ret = 0;
    // 关闭连接后，允许未发完的数据逗留
    ret = setsockopt(m_listenFd, SOL_SOCKET, SO_LINGER, &lingerOpt, sizeof(lingerOpt));
    if (ret < 0) {
        Logger::Instance()->LOG_ERROR("Sock option - linger error");
        return false;
    }

    // 端口复用
    bool reuseFlag = true;
    ret = setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseFlag, sizeof(bool));
    if(ret == -1) {
        Logger::Instance()->LOG_ERROR("Sock option - reuse error");
        return false;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(m_port);
}

void Server::setNonBlocking(int fd) {
    assert(fd);

    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}
