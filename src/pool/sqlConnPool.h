#ifndef _SQL_CONN_POOL_H
#define _SQL_CONN_POOL_H

#include <cstddef>
#include <mysql/mysql.h>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <cassert>

struct SqlConnInfo {
    int port;
    const char* host;
    const char* user;
    const char* pwd;
    const char* db_name;
};

class SqlConnPool {
public:
    SqlConnPool(int conn_nums, SqlConnInfo* info);
    ~SqlConnPool();

public:
    static SqlConnPool* Instance();
    MYSQL* getConn();
    void freeConn(MYSQL* conn);
    void destoryPool();

private:
    int m_conn_nums;
    std::queue<MYSQL*> m_conn_pool;

    std::mutex m_mutex;
    sem_t m_sem;

    int m_used_count;
    int m_freed_count;

    static SqlConnPool s_sqlConnPool;
};

#endif // _SQL_CONN_POOL_H
