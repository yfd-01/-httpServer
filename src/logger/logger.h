#ifndef _LOGGER_H
#define _LOGGER_H

#include "blockingDeque.h"
#include "devices.h"
#include <sys/stat.h>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <cassert>
#include <ctime>

class Logger {
private:
    Logger();
    ~Logger();

public:
    static Logger* Instance();
    static void Destroy();
    void init(MsgLevel level, LoggerDevice device, const char* path, const char* suffix, int dequeCapacity);

    void write(MsgLevel level, const char* msg);
    void write(MsgLevel level, std::string& msg);

private:
    bool m_initilized;
    MsgLevel m_level;
    const char* m_path;
    const char* m_suffix;
    char m_nowTime[64];

    static Logger* s_logger;
    static std::mutex m_mtx;

    std::vector<std::unique_ptr<Device>> m_devices;
    std::unique_ptr<BlockingDeque<std::string>> m_blockingDeq;
    std::unique_ptr<std::thread> m_writeThread;

private:
    void fetchFileName(char* fileName) const;
    void fetchNowTime();
    void writeThreadJobs();
    static void raiseWriteThread();

    void formatMsg(MsgLevel level, std::string& msg);
};

#endif // _LOGGER_H
