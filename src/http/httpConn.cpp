#include "httpConn.h"
#include <cstddef>

ssize_t HttpConn::read(int* readErrno) {
    ssize_t len = -1;

    do {
        size_t writable = m_readBuff.writableBytes();

        m_iovRead[0].iov_base = m_readBuff.beginWritePtr();
        m_iovRead[0].iov_len = writable;
        m_iovRead[1].iov_base = m_expandedBuff;
        m_iovRead[1].iov_len = EXPANDED_BUFF_SIZE;

        const ssize_t len = readv(m_fd, m_iovRead, 2);
        if (len < 0)
            *readErrno = errno;
        else if (len <= writable)
            m_readBuff.hasWritten(len);
        else {
            m_readBuff.beenFilled();
            m_readBuff.append(m_expandedBuff, len - writable);
        }

        if (len == 0)
            break;  // read all

    } while(s_useET);

    return len;
}

ssize_t HttpConn::write(int* readErrno) {
    ssize_t len = -1;

    do {

    } while(s_useET || bytesToSend() > CONTINUE_SEND_BYTES);    // ET模式 或者 待传输数据量大于阈值

    return len;
}


bool HttpConn::process() {
    
}


int HttpConn::getFd() const {
    return m_fd;
}

struct sockaddr_in HttpConn::getAddr() const {
    return m_addr;
}

const char* HttpConn::getIp() const {
    return inet_ntoa(m_addr.sin_addr);
}

int HttpConn::getPort() const {
    return m_addr.sin_port;
}

const int HttpConn::bytesToSend() const {
    return m_iovWrite[0].iov_len + m_iovWrite[1].iov_len;
}

