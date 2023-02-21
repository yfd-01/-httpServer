#include "logger.h"
#include "devices.h"
#include <cassert>
#include <functional>
#include <memory>
#include <thread>

Logger::Logger() {
    m_initilized = false;
    m_level = MsgLevel::_NONE;
    m_path = nullptr;
    m_suffix = nullptr;
    m_time_format = nullptr;
};

Logger::~Logger() {
    //
}

Logger*     Logger::s_logger    = nullptr;
std::mutex  Logger::m_mtx;

Logger* Logger::Instance() {
    if (s_logger == nullptr) {
        {
            std::lock_guard<std::mutex> locker(m_mtx);
            s_logger = new Logger();
        }
    }

    return s_logger;
}

void Logger::init(
    MsgLevel default_level = MsgLevel::_INFO,
    LoggerDevice device = LoggerDevice::_BOTH,
    const char* path = "./log",
    const char* suffix = ".log",
    int dequeCapacity = 1024
) {
    assert(dequeCapacity > 0 && !m_initilized);

    m_level = default_level;
    m_path = path;
    m_suffix = suffix;

    if (!m_blockingDeq)         // blocking deque ready
        m_blockingDeq = std::make_unique<BlockingDeque<std::string>>(dequeCapacity);

    char* fileName;
    fetchFileName(fileName);    // get log file name

    FILE* fp = fopen(fileName, "a");
    if (!fp) {
        mkdir(m_path, 0777);
        fp = fopen(fileName, "a");
    }

    assert(fp != nullptr);      // make log file exists

    std::unique_ptr<Device> device_T = std::make_unique<Terminal>();
    std::unique_ptr<Device> device_F = std::make_unique<File>(fp);

    if (device == LoggerDevice::_TERMINAL)
        m_devices.push_back(std::move(device_T));
    else if (device == LoggerDevice::_FILE)
        m_devices.push_back(std::move(device_F));
    else if (device == LoggerDevice::_BOTH) {
        m_devices.push_back(std::move(device_T));
        m_devices.push_back(std::move(device_F));
    }   // output devices ready

    if (!m_writeThread) 
        m_writeThread = std::make_unique<std::thread>(raiseWriteThread);    // write thread ready

    m_initilized = true;
}

void Logger::write(MsgLevel level, const char* msg) {
    assert(m_initilized);

    if (level > m_level)
        return;

    for (auto& device : m_devices)
        device->write(level, msg);
}

void Logger::write(MsgLevel level, const std::string& msg) {
    assert(m_initilized);

    if (level > m_level)
        return;

    for (auto& device : m_devices)
        device->write(level, msg);
}

void Logger::fetchFileName(char* fileName) const {
    time_t now_time;
    struct tm* t;

    time(&now_time);
    t = localtime(&now_time);

    snprintf(fileName, 256, "%s/%04d_%02d_%02d%s",
        m_path, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, m_suffix);
}

void Logger::writeThreadJobs() {
    std::string msg;

    while (m_blockingDeq->pop(msg)) {
        
    }
}

void Logger::raiseWriteThread() {
    Logger::Instance()->writeThreadJobs();
}
