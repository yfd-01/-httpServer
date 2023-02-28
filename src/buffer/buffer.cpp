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


void Buffer::append(const char* str, size_t len) {
    assert(str);

    ensureWritableBytes(len);
    std::copy(str, str + len, begin() + m_writePos);
    HasWritten(len);
}

void Buffer::append(const std::string& str) {
    append(str.data(), str.length());
}

void Buffer::append(const void* data, size_t len) {
    assert(data);

    append(static_cast<const char*>(data), len);
}

void Buffer::append(const Buffer& buffer) {
    append(buffer.peek(), buffer.readableBytes());
}

void Buffer::ensureWritableBytes(size_t len) {
    if (writableBytes() < len)
        makeSpace(len);

    assert(writableBytes() >= len);
}

void Buffer::makeSpace(size_t len) {
    if (prependableBytes() + writableBytes() < len)
        m_buffer.resize(m_writePos + len);
    else {
        size_t contentSize = readableBytes();
        std::copy(begin() + m_readPos, begin() + m_writePos, begin());
        m_readPos = 0;
        m_writePos = m_readPos + contentSize;

        assert(contentSize == readableBytes());
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
