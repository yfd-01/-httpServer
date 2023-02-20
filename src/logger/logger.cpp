#include "logger.h"

Logger::Logger() = default;

Logger::~Logger() {
    //
}


void Logger::init(
    LoggerLevel level = LoggerLevel::_INFO,
    LoggerDevice device = LoggerDevice::_BOTH,
    const char* path = "./log",
    const char* suffix = ".log",
    int dequeCapacity = 1024
) {
    m_level = level;
    m_device = device;
    m_path = path;
    m_suffix = suffix;

    
}
