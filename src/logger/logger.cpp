#include "logger.h"
#include "devices.h"

Logger::Logger() {
    m_initilized = false;
    m_level = MsgLevel::_NONE;
    m_path = nullptr;
    m_suffix = nullptr;
};

Logger::~Logger() {
    if (m_writeThread && m_writeThread->joinable()) {
        while (!m_blockingDeq->empty()) 
            m_blockingDeq->flush();

        m_writeThread->join();
    }
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

void Logger::Destroy() {
    if (s_logger == nullptr)
        return;
    
    delete s_logger;
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
        
    char fileName[256];
    fetchFileName(fileName);    // get log file name

    std::ofstream ofs;
    ofs.open(fileName, std::ios::app);
    if (!ofs.good()) {
        mkdir(m_path, 0777);
        ofs.open(fileName, std::ios::app);
    }

    assert(ofs.good());         // make log file exists

    std::unique_ptr<Device> device_T = std::make_unique<Terminal>();
    std::unique_ptr<Device> device_F = std::make_unique<File>(ofs);

    if (device == LoggerDevice::_TERMINAL)
        m_devices.push_back(std::move(device_T));
    else if (device == LoggerDevice::_FILE)
        m_devices.push_back(std::move(device_F));
    else if (device == LoggerDevice::_BOTH) {
        m_devices.push_back(std::move(device_T));
        m_devices.push_back(std::move(device_F));
    }   // output devices ready

    if (!m_writeThread) {
        m_writeThread = std::make_unique<std::thread>(raiseWriteThread);    // write thread ready
        m_writeThread->detach();
    }

    m_initilized = true;
}

void Logger::write(MsgLevel level, const char* msg) {
    assert(m_initilized);

    if (level > m_level || level == _NONE)
        return;

    // reformat msg to standard msg
    std::string s_(msg);
    formatMsg(level, s_);
    m_blockingDeq->pushFront(s_);
}

void Logger::write(MsgLevel level, std::string& msg) {
    assert(m_initilized);

    if (level > m_level || level == _NONE)
        return;

    formatMsg(level, msg);
    m_blockingDeq->pushFront(msg);
}

void Logger::formatMsg(MsgLevel level, std::string& msg) {
    msg.append(1, '\n');
    
    switch(level) {
        case _NONE:
            break;
        case _ERROR:
            msg.insert(0, " [ERROR]: ");
            break;
        case _WARNING:
            msg.insert(0, " [WARNING]: ");
            break;
        case _DEBUG:
            msg.insert(0, " [DEBUG]: ");
            break;
        default:
            msg.insert(0, " [INFO]: ");
            break;
    }

    fetchNowTime();
    msg.insert(0, m_nowTime);
}

void Logger::fetchFileName(char* fileName) const {
    time_t now_time;
    struct tm* t;

    time(&now_time);
    t = localtime(&now_time);

    snprintf(fileName, 256, "%s/%04d_%02d_%02d%s",
        m_path, t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, m_suffix);
}

void Logger::fetchNowTime() {
    time_t now_time;
    struct tm* t;

    time(&now_time);
    t = localtime(&now_time);

    snprintf(m_nowTime, 56, "%04d-%02d-%02d %02d:%02d:%02d %s",
    t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, t->tm_zone);
}

void Logger::writeThreadJobs() {
    std::string msg;

    while (m_blockingDeq->pop(msg)) {
        std::lock_guard<std::mutex> locker(m_mtx);

        for (auto& device : m_devices)
            device->write(msg);
    }
}

void Logger::raiseWriteThread() {
    Logger::Instance()->writeThreadJobs();
}
