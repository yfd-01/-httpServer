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

#define LOG_FILE_NAME_MAX_LEN 256
#define NOW_TIME_STR_MAX_LEN 64

struct LogFileDate {
    int year;
    int month;
    int day;

    bool operator== (const LogFileDate& d) const {
        return year == d.year && month == d.month && day == d.day;
    }

    bool operator!= (const LogFileDate& d) const {
        return !(*this == d);
    }
};

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
    LoggerDevice m_device;
    MsgLevel m_level;
    const char* m_path;
    const char* m_suffix;
    LogFileDate m_new_date, m_old_date;
    char m_nowTime[NOW_TIME_STR_MAX_LEN];

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
    void openLogFile(std::ofstream& ofs, const char* fileName);

public:
    void LOG_ERROR(const char* msg);
    void LOG_ERROR(std::string& msg);
    void LOG_WARNING(const char* msg);
    void LOG_WARNING(std::string& msg);
    void LOG_DEBUG(const char* msg);
    void LOG_DEBUG(std::string& msg);
    void LOG_INFO(const char* msg);
    void LOG_INFO(std::string& msg);
};

#endif // _LOGGER_H
