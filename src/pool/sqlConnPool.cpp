#include "sqlConnPool.h"

SqlConnPool SqlConnPool::s_sqlConnPool;

void SqlConnPool::init(int conn_nums, SqlConnInfo* info) {
    assert(conn_nums > 0);

    for (int i = 0; i < conn_nums; i++) {
        MYSQL* conn = nullptr;
        conn = mysql_init(conn);

        if(!conn) {
            Logger::Instance()->LOG_ERROR("数据库连接实例启动失败 - 1");
            exit(-2);
        }
        conn = mysql_real_connect(conn, info->host, info->user, info->pwd, info->db_name, info->port, nullptr, 0);

        if (!conn) {
            Logger::Instance()->LOG_ERROR("数据库连接实例启动失败 - 2");
            exit(-2);
        }

        m_conn_pool.push(conn);
    }
    
    m_conn_nums = conn_nums;
    m_freed_count = conn_nums;
    sem_init(&m_sem, 0, m_conn_nums);
    
    Logger::Instance()->LOG_INFO("数据库连接池启动成功");
}

SqlConnPool::~SqlConnPool() {
    destoryPool();
}

SqlConnPool* SqlConnPool::Instance() {
    return &s_sqlConnPool;
}


MYSQL* SqlConnPool::getConn() {
    MYSQL* conn = nullptr;

    // if (m_conn_pool.empty()) {
    //     // LOG_WARNNING()
    //     return nullptr;
    // }

    sem_wait(&m_sem);

    {
        std::lock_guard<std::mutex> locker(m_mutex);
        conn = m_conn_pool.front();
        m_conn_pool.pop();

        m_freed_count--;
        m_used_count++;
    }

    return conn;
}

void SqlConnPool::freeConn(MYSQL* conn) {
    std::lock_guard<std::mutex> locker(m_mutex);
    m_conn_pool.push(conn);
    
    m_used_count--;
    m_freed_count++;

    sem_post(&m_sem);
}

void SqlConnPool::destoryPool() {
    std::lock_guard<std::mutex> locker(m_mutex);

    MYSQL* to_freed_conn = nullptr;

    while (!m_conn_pool.empty()) {
        to_freed_conn = m_conn_pool.front();
        mysql_close(to_freed_conn);

        m_conn_pool.pop();
    }

    Logger::Instance()->LOG_INFO("数据库连接池销毁");
}
