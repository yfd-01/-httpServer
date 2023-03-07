#include "server.h"

const int Server::MAX_FD = 65535;
short Server::s_forceQuit = 0;

/**
 * @brief Construct a new Server:: Server object
 * 
 * @param baseConfig    服务器基础配置
 * @param sqlConfig     数据库配置
 * @param loggerConfig  日志系统配置
 * @param threadNums    事务线程数量
 * @param sqlConnNums   数据库连接池中连接实例数量
 * @param loggerQueSize 日志系统阻塞队列大小
 */
Server::Server(
    BaseConfig* baseConfig, SQLConfig* sqlConfig, LoggerConfig* loggerConfig,
    int threadNums, int sqlConnNums, int loggerQueSize
) {
    char* srcDir = getcwd(nullptr, 256);

    HttpConn::s_usersCount = 0;
    HttpConn::s_srcDir = std::string(srcDir) + "/static";  // 静态资源路径

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
    if (!initialize(baseConfig->_lingerUsing)) {
        close(m_listenFd);
        Logger::Instance()->LOG_ERROR("服务器启动失败");

        serverShutdown();
        exit(-1);
    }

    depictServerInit(baseConfig->_lingerUsing, threadNums, sqlConnNums, loggerQueSize);
    signal(SIGINT, Server::interruptionHandler);    // 退出信号捕获
}

Server::~Server() {
    serverShutdown();
}

/**
 * @brief 
 */
void Server::run() {
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
        // if (m_users.count(fd) == 0) 
        //     m_users.emplace(fd, new HttpConn());

        m_users[fd].init(fd, addr);

        if (m_timeoutMS > 0) 
            // 访问unordered_map没有的key会默认调用无参构造
            m_timer->add(fd, m_timeoutMS, std::bind(&Server::handleClose, this, &m_users[fd]));

        m_epoller->addFd(fd, EPOLLIN | m_connEvents);

        setNonBlocking(fd);
        std::string msg = "Client - " + std::to_string(fd) + " conn in";
        Logger::Instance()->LOG_INFO(msg);
        msg = "current online users: " + std::to_string(HttpConn::s_usersCount);
        Logger::Instance()->LOG_INFO(msg);

    } while (m_listenEvents & EPOLLET); // accept all if listen mode is ET
}

void Server::handleClose(HttpConn* conn) {
    assert(conn);

    if (conn->doClose()) {
        std::string msg = "Client - " + std::to_string(conn->getFd()) + " conn close";
        Logger::Instance()->LOG_INFO(msg);
        msg = "current online users: " + std::to_string(HttpConn::s_usersCount);
        Logger::Instance()->LOG_INFO(msg);
    }

    m_epoller->delFd(conn->getFd());
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

bool Server::initialize(bool lingerUsing) {
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
    int reuseFlag = 1;
    ret = setsockopt(m_listenFd, SOL_SOCKET, SO_REUSEADDR, (const void*)&reuseFlag, sizeof(int));
    if(ret < 0) {
        Logger::Instance()->LOG_ERROR("Sock option - reuse error");
        return false;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(m_port);

    ret = bind(m_listenFd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        Logger::Instance()->LOG_ERROR("Bind socket error");
        return false;
    }

    ret = listen(m_listenFd, 6);
    if (ret < 0) {
        Logger::Instance()->LOG_ERROR("Listen socket error");
        return false;
    }

    if (!m_epoller->addFd(m_listenFd, m_listenEvents | EPOLLIN)) {
        Logger::Instance()->LOG_ERROR("Add listenFd into epoll error");
        return false;
    }

    setNonBlocking(m_listenFd);
    Logger::Instance()->LOG_INFO("Server initialization done");

    return true;
}

void Server::interruptionHandler(int signal) {
    if (!s_forceQuit) {
        s_forceQuit += 1;

        Logger::Instance()->LOG_INFO("Quiting...");
        Logger::Instance()->Destroy();  // flush all remainings
    }

    // 强制退出
    exit(-1);
}

void Server::serverShutdown() {
    close(m_listenFd);

    // for (auto& pair : m_users) {
    //     close(pair.first);
    //     delete pair.second;
    // }

    SqlConnPool::Instance()->destoryPool();
    Logger::Instance()->LOG_INFO("服务器关闭");
    Logger::Instance()->Destroy();
}

void Server::setNonBlocking(int fd) {
    assert(fd);

    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
}

void Server::depictServerInit(bool lingerUsing, int threadNums, int sqlConnNums, int loggerQueSize) const {
    assert(Logger::Instance());

    std::string msg = "";
    Logger* logger = Logger::Instance();
    logger->LOG_INFO("----------------------HttpServer-------------------------");
    
    msg = "端口: " + std::to_string(m_port) + "   自动断开超时时延: " + std::to_string(m_timeoutMS) + " ms" + "   是否开启连接逗留: " + (lingerUsing ? "是" : "否");
    logger->LOG_INFO(msg);

    msg = std::string("listenFdMode: ") + (m_listenEvents & EPOLLET ? "ET" : "LT");
    msg += std::string("   connFdMode: ") + (m_connEvents & EPOLLET ? "ET" : "LT");
    logger->LOG_INFO(msg);

    msg = "线程池中线程数量: " + std::to_string(threadNums) + "   数据库连接池中实例数量: " + std::to_string(sqlConnNums);
    logger->LOG_INFO(msg);

    auto [levelStr, deviceStr, pathStr] = logger->loggerDesc();
    msg = "日志系统等级: " + levelStr + "   日志记录形式: " + deviceStr;
    if (deviceStr != "仅终端")
        msg += "   日志文件目录: " + pathStr;
    logger->LOG_INFO(msg);

    logger->LOG_INFO("---------------------------------------------------------");
}

/**
 * @brief 描述服务器当前状态
 */
void Server::depictServerStatus() const {
    
}
