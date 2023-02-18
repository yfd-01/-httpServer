#include "pool/sqlConnPool.h"
#include "pool/threadPool.h"
#include <iostream>
#include <functional>
#include <unistd.h>
#include <string>

#define SQLCONNPOOL_TEST 0
#define THREADPOOL_TEST 1

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
        void (*print_func_ptr)(void) = [] {
            std::cout<< "info"<< std::endl;
        };

        ThreadPool tp(6);

        for (int i = 0; i < 20; i++) {
            tp.addTask(print_func_ptr);

            if (i == 10) {
                sleep(2);
            }else if(i == 15) {
                sleep(1);
            }
        }
        sleep(1);
    }
#endif

    std::cin.get();

    return 0;
}
