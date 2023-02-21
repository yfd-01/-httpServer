#include "devices.h"

// Terminal::Terminal() {};

void Terminal::write(MsgLevel level, const char *msg) {
    std::cout<< msg;
}

void Terminal::write(MsgLevel level, const std::string& msg) {
    std::cout<< msg;
}
