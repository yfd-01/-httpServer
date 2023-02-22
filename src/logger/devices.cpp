#include "devices.h"


void Terminal::write(const char* msg) {
    std::cout<< msg;
}

void Terminal::write(const std::string& msg) {
    std::cout<< msg;
}


File::File(std::ofstream& ofs) {
    m_ofs = std::move(ofs);
}

File::~File() {
    m_ofs.flush();
    m_ofs.close();
}

void File::write(const char *msg) {
    m_ofs<< msg;
    // m_ofs.flush();
}

void File::write(const std::string& msg) {
    m_ofs<< msg;
    // m_ofs.flush();
}
