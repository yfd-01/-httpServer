#ifndef _DEVICES_H
#define _DEVICES_H

#include <bits/types/FILE.h>
#include <iostream>

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
    Device();
    virtual ~Device();

public:
    virtual void write(MsgLevel level, const char* msg) = 0;
    virtual void write(MsgLevel level, const std::string& msg) = 0;
};

class Terminal: public Device {
public:
    Terminal();

public:
    void write(MsgLevel level, const char* msg);
    void write(MsgLevel level, const std::string& msg);
};

class File: public Device {
public:
    File(FILE* fp);

public:
    void write(MsgLevel level, const char* msg);
    void write(MsgLevel level, const std::string& msg);

private:
    const FILE* fp;
};

#endif  // _DEVICES_H