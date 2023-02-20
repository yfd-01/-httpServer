#include "pool/sqlConnPool.h"
#include "pool/threadPool.h"
#include "timer/heapTimer.h"
#include <iostream>
#include <functional>
#include <unistd.h>
#include <string>

#define SQLCONNPOOL_TEST 0
#define THREADPOOL_TEST 0
#define HEAPTIMER_TEST 0

void func() {
    std::cout<< "hello: "<< std::endl;
}

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

    std::cin.get();

    return 0;
}
