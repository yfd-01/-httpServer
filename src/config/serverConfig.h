#ifndef _SERVER_CONFIG_H
#define _SERVER_CONFIG_H

#include "../pool/sqlConnPool.h"
#include "../logger/devices.h"

/**
 * @brief 数据库配置
 */
typedef SqlConnInfo SQLConfig;

/**
 * @brief 基础配置
 */
struct BaseConfig {
    int _port;
    short _modeChoice;
    int _timeoutMS;
    bool _lingerUsing;

    BaseConfig() {
        _port = 7777;
        _modeChoice = 3;
        _timeoutMS = 60000;
        _lingerUsing = true;
    }

    BaseConfig(int port, short modeChoice, int timeoutMS, bool lingerUsing)
        :_port(port), _modeChoice(modeChoice), _timeoutMS(timeoutMS), _lingerUsing(lingerUsing) {}
};

/**
 * @brief 日志配置
 */
struct LoggerConfig {
    MsgLevel _level;
    LoggerDevice _device;
    const char* _path;
    const char* _suffix;

    LoggerConfig() {
        _level = _INFO;
        _device = _BOTH;
        _path = "./log";
        _suffix = ".log";
    }

    LoggerConfig(MsgLevel level, LoggerDevice device, const char* path, const char* suffix)
        :_level(level), _device(device), _path(path), _suffix(suffix) {}
};

#endif  // _SERVER_CONFIG_H