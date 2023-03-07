#include "server/server.h"

int main() {
    BaseConfig baseConfig = { 7777, 3, 60000, 1 };
    SQLConfig sqlConfig = { 3306, "127.0.0.1", "root", "123", "http" };
    LoggerConfig loggerConfig = { _INFO, _BOTH, "./log", ".log" };

    Server httpServer(&baseConfig, &sqlConfig, &loggerConfig, 8, 16, 1024);

    httpServer.run();

    return 0;
}
