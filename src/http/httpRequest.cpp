#include "httpRequest.h"

std::unordered_set<std::string> HttpRequest::DEFAULT_HTML {
    "index", "login", "register", "welcome", "picture", "video", "error"
};

void HttpRequest::init() {
    m_requestInfo = std::make_unique<RequestInfo>();
    m_parsePhase = _REQUEST_LINE;
}

/**
 * @brief 解析http请求
 */
bool HttpRequest::parse(Buffer& buff) {
    if (buff.readableBytes() <= 0)
        return false;

    const char CRLF[] = "\r\n";
    
    while (buff.readableBytes() && m_parsePhase < _FINISH) {
        const char* line = std::search(buff.peek(), buff.peek() + buff.readableBytes(), CRLF, CRLF + 2);
        std::string lineStr(buff.peek(), line);

        switch(m_parsePhase) {
            case _REQUEST_LINE:
                if (!_parseRequestLine(lineStr)) return false;
                _parsePath();
                break;
            case _HEADERS:
                _parseHeaders(lineStr);
                break;
            case _BODY:
                _parseBody(lineStr);
                break;
            default:
                break;
        }
        if (line >= buff.peek() + buff.readableBytes()) break;
        buff.retrieveUntil(line + 2);
    }

    return true;
}

bool HttpRequest::_parseRequestLine(const std::string& lineStr) {
    std::regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    std::smatch matchRes;

    if (regex_match(lineStr, matchRes, patten)) {
        m_requestInfo->method = matchRes[1];
        m_requestInfo->path = matchRes[2];
        m_requestInfo->version = matchRes[3];

        m_parsePhase = _HEADERS;
        return true;
    }
    Logger::Instance()->LOG_ERROR("happened error in parsing requestLine");

    return false;
}

void HttpRequest::_parsePath() {
    if (m_requestInfo->path == "/")
        m_requestInfo->path = "/index.html";
    else {
        for (auto& dh : DEFAULT_HTML) {
            if (dh == m_requestInfo->path) {
                m_requestInfo->path += ".html";
                break;
            }
        }
    }
}

void HttpRequest::_parseHeaders(const std::string& lineStr) {
    std::regex pattern("^([^:]*): ?(.*)$");
    std::smatch matchRes;

    if (regex_match(lineStr, matchRes, pattern)) 
        m_requestInfo->headers[matchRes[1]] = matchRes[2];
    else
        m_parsePhase = _BODY;
}

void HttpRequest::_parseBody(const std::string& lineStr) {
    if (m_requestInfo->method == "POST" && m_requestInfo->headers["Content-Type"] == "application/x-www-form-urlencoded" && lineStr.length()) {
        m_requestInfo->body = lineStr;
        _urlDecode();

        bool flag = false;
        std::string user = m_requestInfo->postData["username"];
        std::string password = m_requestInfo->postData["password"];

        if (m_requestInfo->postData["isLogin"] == "1")
            flag = userLogin(user, password);
        else
            flag = userRegister(user, password);

        m_requestInfo->path = flag ? "welcome.html" : "error.html";
    }

    m_parsePhase = _FINISH;
}

void HttpRequest::_urlDecode() {
    std::string tmp = "";
    int length = m_requestInfo->body.length();

    for (int i = 0; i < length; i++) {
        char ch = m_requestInfo->body[i];
        
        if (ch == '+')
            tmp += ' ';
        else if (ch == '%') {
            assert(i + 2 < length);
            char high = _fromChar(m_requestInfo->body[++i]);
            char low = _fromChar(m_requestInfo->body[++i]);

            tmp += high * 16 + low;
        } else
            tmp += ch;
    }

    m_requestInfo->body = tmp;
    
    // post data resolved 
    std::string key, value, msg;
    int l = 0, r = 0;
    int len = m_requestInfo->body.length();

    for (; r < len; r++) {
        char ch = m_requestInfo->body[r];

        switch (ch) {
            case '=':
                key = m_requestInfo->body.substr(l, r - l);
                l = r + 1;
                break;
            case '&':
                value = m_requestInfo->body.substr(l, r - l);
                l = r + 1;
                m_requestInfo->postData[key] = value;

                msg = key + " = " + value;
                Logger::Instance()->LOG_DEBUG(msg);
                break;
            default:
                break;
        }
    }

    if (m_requestInfo->postData.count(key) == 0 && r > l) 
        m_requestInfo->postData[key] = m_requestInfo->body.substr(l, r - l);
}

char HttpRequest::_fromChar (char ch) { 
    char ret;
    if (ch >= 'A' && ch <= 'Z') ret = ch - 'A' + 10;
    else if (ch >= 'a' && ch <= 'z') ret = ch - 'a' + 10;
    else if (ch >= '0' && ch <= '9') ret = ch - '0';
    else assert(0);

    return ret;
}

bool HttpRequest::userLogin(const std::string& usr, const std::string& psw) {
    bool flag = false;

    if (usr == "" || psw == "") return flag;
    
    std::string msg = "check login: " + usr + " && " + psw;
    Logger::Instance()->LOG_DEBUG(msg);

    MYSQL* conn = SqlConnPool::Instance()->getConn();
    assert(conn);

    char sqlStr[256];
    MYSQL_RES* res = nullptr;

    snprintf(sqlStr, 256, "SELECT username, password FROM login Where username = '%s' LIMIT 1", usr.data());

    msg = "check sql: " + std::string(sqlStr);
    Logger::Instance()->LOG_DEBUG(msg);

    if (mysql_query(conn, sqlStr)) {
        mysql_free_result(res);
        return false;
    }

    res = mysql_store_result(conn);

    while (MYSQL_ROW row = mysql_fetch_row(res)) {
        std::string psw_(row[1]);

        flag = psw == psw_;
    }

    return flag;
}

bool HttpRequest::userRegister(const std::string& usr, const std::string& psw) {
    bool flag = false;

    if (usr == "" || psw == "") return flag;
    
    std::string msg = "register: " + usr + " && " + psw;
    Logger::Instance()->LOG_DEBUG(msg);

    MYSQL* conn = SqlConnPool::Instance()->getConn();
    assert(conn);

    char sqlStr[256];
    MYSQL_RES* res = nullptr;

    snprintf(sqlStr, 256, "SELECT username FROM login Where username = '%s' LIMIT 1", usr.data());

    msg = "sql: " + std::string(sqlStr);
    Logger::Instance()->LOG_DEBUG(msg);

    if (mysql_query(conn, sqlStr)) {
        mysql_free_result(res);
        return false;
    }

    res = mysql_store_result(conn);
    if (res->row_count == 0) {
        bzero(sqlStr, 256);
        snprintf(sqlStr, 256, "INSERT INTO login VALUES('%s', '%s')", usr.data(), psw.data());

        std::string msg = "register sql: " + usr + " && " + psw;
        Logger::Instance()->LOG_INFO(msg);

        if (mysql_query(conn, sqlStr)) {
            Logger::Instance()->LOG_DEBUG("register error");
            flag =false;
        }

        flag = true;
    }

    SqlConnPool::Instance()->freeConn(conn);

    return flag;
}


std::string HttpRequest::method() const {
    return m_requestInfo->method;
}

std::string HttpRequest::path() const {
    return m_requestInfo->path;
}

std::string HttpRequest::version() const {
    return m_requestInfo->version;
}

bool HttpRequest::isKeepAlive() const {
    if (m_requestInfo->headers.count("Connection") == 1)
        return m_requestInfo->headers.find("Connection")->second == "keep-alive" && m_requestInfo->version == "1.1";

    return false;
}
