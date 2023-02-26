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
    virtual void changeOFS(std::ofstream& new_ofs) {}
public:
    LoggerDevice type_;
};

class Terminal: public Device {
public:
    Terminal();

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

    void changeOFS(std::ofstream& new_ofs);
private:
    std::ofstream m_ofs;
};

#endif  // _DEVICES_H