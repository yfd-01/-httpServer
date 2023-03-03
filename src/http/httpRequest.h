#ifndef _HTTP_REQUEST_H
#define _HTTP_REQUEST_H

#include <memory>
#include <string>
#include <regex>
#include <unordered_map>
#include <unordered_set>

#include "mysql/mysql.h"
#include "../pool/sqlConnPool.h"
#include "../buffer/buffer.h"
#include "../logger/logger.h"

class HttpRequest {
public:
    enum PARSE_PHASE {
        _REQUEST_LINE,
        _HEADERS,
        _BODY,
        _FINISH
    };

    void init();

    bool parse(Buffer& buff);
    std::string method() const;
    std::string version() const;
    std::string path() const;

    bool isKeepAlive() const;

private:
    struct RequestInfo {
        std::string method;
        std::string path;
        std::string version;
        std::string body;

        std::unordered_map<std::string, std::string> headers;
        std::unordered_map<std::string, std::string> postData;

        RequestInfo() {
            method = path = version = body = "";
            headers.clear();
            postData.clear();
        }
    };

private:
    PARSE_PHASE m_parsePhase;
    std::unique_ptr<RequestInfo> m_requestInfo;

    bool _parseRequestLine(const std::string& str);
    void _parsePath();
    void _parseHeaders(const std::string& str);
    void _parseBody(const std::string& str);
    void _urlDecode();
    char _fromChar(char ch);

    bool userLogin(const std::string& user, const std::string& psw);
    bool userRegister(const std::string& user, const std::string& psw);

    static std::unordered_set<std::string> DEFAULT_HTML;
};

#endif  // _HTTP_REQUEST_H
