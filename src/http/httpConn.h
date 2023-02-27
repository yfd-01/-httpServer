#ifndef _HTTP_CONN_H
#define _HTTP_CONN_H

#include <netinet/in.h>
#include <sys/types.h>

class HttpConn {
public:
    HttpConn();

public:
    void close();
    int getFd();
    ssize_t read();
    
    static bool s_useET;
private:
    int m_fd;
    struct sockaddr_in addr;
};

#endif  // _HTTP_CONN_H