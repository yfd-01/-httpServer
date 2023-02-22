#ifndef _DEVICES_H
#define _DEVICES_H

#include <iostream>
#include <fstream>

enum MsgLevel {
    _NONE,
    _ERROR,
    _WARNING,
    _DEBUG,
    _INFO,
};

enum LoggerDevice {
    _TERMINAL,
    _FILE,
    _BOTH
};

class Device {
public:
    virtual ~Device() = default;

public:
    virtual void write(const char* msg) = 0;
    virtual void write(const std::string& msg) = 0;
};

class Terminal: public Device {
public:
    void write(const char* msg);
    void write(const std::string& msg);
};

class File: public Device {
public:
    File(std::ofstream& ofs);
    ~File();

public:
    void write(const char* msg);
    void write(const std::string& msg);

private:
    std::ofstream m_ofs;
};

#endif  // _DEVICES_H