#include "buffer.h"
#include <cassert>

Buffer::Buffer(int initialSize): m_readPos(0), m_writePos(0) {
    m_buffer.reserve(initialSize);
}

size_t Buffer::readableBytes() const {
    return m_writePos - m_readPos;
}

size_t Buffer::writableBytes() const {
    return m_buffer.size() - m_writePos;
}

size_t Buffer::prependableBytes() const {
    return m_readPos;
}


void Buffer::retrieve(size_t len) {
    assert(len <= readableBytes());

    m_readPos += len;
}

void Buffer::retrieveUntil(const char *end) {
    assert(peek() <= end);

    retrieve(end - peek());
}

void Buffer::retrieveAll() {
    bzero(&m_buffer[0], m_buffer.size());
    m_readPos = 0;
    m_writePos = 0;
}


void append(const char* str, size_t len) {
    assert(str);


}
void append(const std::string& str);
void append(const void* data, size_t len);
void append(const Buffer& buffer);

void Buffer::ensureWritableBytes(size_t len) {
    if (writableBytes() < len)
        makeSpace(len);

    assert(writableBytes() >= len);
}

void Buffer::makeSpace(size_t len) {
    if (prependableBytes() + writableBytes() < len)
        m_buffer.resize(m_writePos + len);
    else {
        
    }
}


const char* Buffer::peek() const {
    return begin() + m_readPos;
}

const char* Buffer::begin() const {
    return &*m_buffer.begin();
}

char* Buffer::begin() {
    return &*m_buffer.begin();
}
