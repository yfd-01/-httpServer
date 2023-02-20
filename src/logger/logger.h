#ifndef _LOGGER_H
#define _LOGGER_H

#include "blockingDeque.h"
#include <memory>
#include <mutex>
#include <thread>

enum LoggerLevel {
    _NONE,
    _ERROR,
    _WARNING,
    _DEBUG,
    _INFO,
};

enum LoggerDevice {
    _TERMINAL,
    _FILE,
    _BOTH
};

class Logger {
private:
    Logger();
    ~Logger();

public:
    static Logger* Instance();
    void init(LoggerLevel level, LoggerDevice device, const char* path, const char* suffix, int dequeCapacity);

private:
    LoggerLevel m_level;
    LoggerDevice m_device;
    const char* m_path;
    const char* m_suffix;

    FILE* fp;
    static Logger* s_logger;
    std::mutex m_mtx;

    std::unique_ptr<BlockingDeque<std::string>> m_blockingDeq;
    std::unique_ptr<std::thread> m_writeThread;
};

#endif // _LOGGER_H
