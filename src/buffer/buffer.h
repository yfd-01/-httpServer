#ifndef _BUFFER_H
#define _BUFFER_H

#include <atomic>
#include <vector>
#include <cassert>
#include <strings.h>

class Buffer {
public:
    Buffer(int initialSize=1024);
    ~Buffer();

public:
    size_t readableBytes() const;     // 可读字节
    size_t writableBytes() const;     // 可写字节
    size_t prependableBytes() const;  // 前置字节

    const char* peek() const;
    void retrieve(size_t len);
    void retrieveUntil(const char* end);
    void retrieveAll();

    void hasWritten(size_t len);
    void beenFilled();

    void append(const char* str, size_t len);
    void append(const std::string& str);
    void append(const void* data, size_t len);
    void append(const Buffer& buffer);

    char* beginWritePtr();

private:
    std::vector<char> m_buffer;
    std::atomic<size_t> m_readPos;
    std::atomic<size_t> m_writePos;

    const char* begin() const;
    char* begin();

    void ensureWritableBytes(size_t len);
    void makeSpace(size_t len);
    void HasWritten(size_t len);
};

#endif  // _BUFFER_H