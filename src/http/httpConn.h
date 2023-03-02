#ifndef _HTTP_CONN_H
#define _HTTP_CONN_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <errno.h>
#include <sys/unistd.h>

#include "../buffer/buffer.h"
#include "httpRequest.h"
#include "httpResponse.h"

#define EXPANDED_BUFF_SIZE  65535
#define CONTINUE_SEND_BYTES 10240

class HttpConn {
public:
    HttpConn();

public:
    void init(int connFd, const sockaddr_in& addr);
    ssize_t read(int* readErrno);
    ssize_t write(int* writeErrno);
    
    static bool s_useET;
    static std::string s_srcDir;
    static std::atomic<size_t> s_usersCount;

public:
    int getFd() const;
    struct sockaddr_in getAddr() const;

    const char* getIp() const;
    int getPort() const;

    bool process();
    void doClose();

    const int bytesToSend() const;
    const bool isKeepAlive() const;
    
private:
    int m_fd;
    struct sockaddr_in m_addr;

    Buffer m_readBuff;
    Buffer m_writeBuff;

    char m_expandedBuff[EXPANDED_BUFF_SIZE];

    struct iovec m_iovRead[2];
    struct iovec m_iovWrite[2];
    short m_iovWriteCnt;

    HttpRequest m_request;
    HttpResponse m_response;
};

#endif  // _HTTP_CONN_H