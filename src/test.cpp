#include "logger/devices.h"
#include "pool/sqlConnPool.h"
#include "pool/threadPool.h"
#include "timer/heapTimer.h"
#include "logger/logger.h"
#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <functional>
#include <unistd.h>
#include <string>
#include <vector>

#define SQLCONNPOOL_TEST    0   // 数据库连接池测试
#define THREADPOOL_TEST     0   // 线程池测试
#define HEAPTIMER_TEST      0   // 最小时间堆测试
#define BLOCKINGDEQUE_TEST  0   // 阻塞队列测试
#define LOGGER_TEST         0   // 日志测试

void func() {
    std::cout<< "hello: "<< std::endl;
}

// 计时器
class Timer {
public:
    Timer() {
        start = std::chrono::high_resolution_clock::now();
    }

    ~Timer() {
        end = std::chrono::high_resolution_clock::now();
        duration = end - start;

        std::cout<< "Timer took: "<< duration.count() * 1000.0f<< "ms\n";
    }

private:
    std::chrono::system_clock::time_point start, end;
    std::chrono::duration<float> duration;
};

int main() {
#if SQLCONNPOOL_TEST
    {
        SqlConnInfo sci({
            3306,
            "47.108.177.180",
            "root",
            "yfd_code_mysql",
            "words"
        });

        SqlConnPool scp(12, &sci);
        std::cout<< scp.getConn()<< std::endl;
    }
#endif

#if THREADPOOL_TEST
    {
        auto print_func_ptr = [](const char* info) {
            std::cout<< "info: "<< info<< std::endl;
        };

        ThreadPool tp(6);

        for (int i = 0; i < 20; i++) {
            tp.addTask(std::bind(print_func_ptr, "yfd"));
        }
        sleep(1);
    }
#endif

#if HEAPTIMER_TEST
    {
        HeapTimer timer;
        timer.add(10, 2000, func);
        timer.add(12, 8000, func);
        timer.add(15, 1500, func);
        sleep(1);
        std::cout<< timer.getNextTick()<< std::endl;
        sleep(1);
        timer.fresh();
    }
#endif

#if BLOCKINGDEQUE_TEST
    {
        BlockingDeque<std::string> deq(3);
        
        int thread_nums = 5;
        std::vector<std::thread> threads;

        for (int i = 0; i < thread_nums; i++) {
            std::thread t([&deq] {
                deq.pushFront("yfd1");
                deq.pushFront("yfd2");
            });

            threads.push_back(std::move(t));
        }
        std::string str;

        std::cout<< deq.size()<< std::endl;
        sleep(1);
        std::cout<< "pop1: "<< deq.pop(str)<< std::endl;
        deq.clear();
        std::cout<< "pop2: "<<deq.pop(str, 1000)<< std::endl;;
        sleep(2);
        std::cout<< deq.size()<< std::endl;
        std::cout<< "pop3: "<<deq.pop(str)<< std::endl;;
        std::cout<< deq.size()<< std::endl;

        sleep(1);
        // deq.feed();
        // deq.feed();
        // deq.feed();
        std::cout<< deq.size()<< std::endl;

        for (auto& t: threads)
            t.join();
    }
#endif

#if LOGGER_TEST
    {
        // Logger::Instance()->write(LoggerLevel::_INFO, "hello\n");
        // std::cout<< Logger::Instance()<< std::endl;
        // std::cout<< Logger::Instance()<< std::endl;
        // std::cout<< Logger::Instance()<< std::endl;

        // Logger::Instance()->init(MsgLevel:: _DEBUG, LoggerDevice::_BOTH, "./log", ".log", 1024);

        // Logger::Instance()->write(MsgLevel::_INFO, "hello from logger1!");
        // Logger::Instance()->write(MsgLevel::_WARNING, "hello from logger2!");
        // Logger::Instance()->write(MsgLevel::_NONE, "hello from logger3!");
        // Logger::Instance()->write(MsgLevel::_DEBUG, "hello from logger4!");
        // Logger::Instance()->write(MsgLevel::_ERROR, "hello from logger5!");
        // ----------------------------------------------------------------------
        
        Logger::Instance()->init(MsgLevel::_INFO, LoggerDevice::_FILE, "./log", ".log", 1024);

        {
            Timer timer;

            for (int i = 0; i < 999999; i++)
                Logger::Instance()->write(MsgLevel::_INFO, "hello from logger1!");
                // Logger::Instance()->LOG_DEBUG("hello from logger1! DEBUG");

            std::cout<< "DONE\n";   // 6107.84ms 5893.32ms
        }

        // ----------------------------------------------------------------------
        // Logger::Instance()->init(MsgLevel::_INFO, LoggerDevice::_BOTH, "./log", ".log", 1024);
        // Logger::Instance()->write(_INFO, "aaa");
    }
#endif
    int i = -1;
    if (i > strlen("hello")) {
        std::cout<< "wwwwwwwwwwwwwwwww\n";
    }else{
        std::cout<< "YYYYYYYYYYYYYYYYYYYYYY\n";
    }

    std::cin.get();

#if LOGGER_TEST
    Logger::Destroy();
#endif

    return 0;
}
