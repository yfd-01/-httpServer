#ifndef _HTTP_CONN_H
#define _HTTP_CONN_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <errno.h>
#include "../buffer/buffer.h"

#define EXPANDED_BUFF_SIZE  65535
#define CONTINUE_SEND_BYTES 10240

class HttpConn {
public:
    HttpConn();

public:
    ssize_t read(int* readErrno);
    ssize_t write(int* writeErrno);
    
    static bool s_useET;

public:
    int getFd() const;
    struct sockaddr_in getAddr() const;

    const char* getIp() const;
    int getPort() const;

    bool process();

private:
    int m_fd;
    struct sockaddr_in m_addr;

    Buffer m_readBuff;
    Buffer m_writeBuff;

    char m_expandedBuff[EXPANDED_BUFF_SIZE];

    struct iovec m_iovRead[2];
    struct iovec m_iovWrite[2];

private:
    const int bytesToSend() const;
};

#endif  // _HTTP_CONN_H