#include "httpResponse.h"

const std::unordered_map<std::string, std::string> HttpResponse::SUFFIX_TYPE = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};

const std::unordered_map<int, std::string> HttpResponse::CODE_STATUS = {
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

const std::unordered_map<int, std::string> HttpResponse::CODE_ERR_PATH = {
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};

HttpResponse::~HttpResponse() {
    unmapFile();
}

void HttpResponse::init(std::string srcDir, std::string path, bool isKeepAlive, int code) {
    unmapFile();

    m_srcDir = srcDir;
    m_path = path;
    m_isKeepAlive = isKeepAlive;
    m_code = code;
    m_fileState = { 0 };
}

void HttpResponse::makeResponse(Buffer& buff) {

    if (stat((m_srcDir + m_path).data(), &m_fileState) < 0 || S_ISDIR(m_fileState.st_mode))
        m_code = 404;
    else if (!(m_fileState.st_mode & S_IROTH))
        m_code = 403;
    else
        m_code = 200;

    if (CODE_ERR_PATH.count(m_code)) {
        m_path = CODE_ERR_PATH.find(m_code)->second;
        stat((m_srcDir + m_path).data(), &m_fileState);
    }

    addStatusLine(buff);
    addHeaders(buff);
    addContent(buff);
}

void HttpResponse::addStatusLine(Buffer& buff) {
    std::string status;

    if (CODE_STATUS.count(m_code))
        status = CODE_STATUS.find(m_code)->second;
    else {
        m_code = 400;
        status = CODE_STATUS.find(m_code)->second;
    }

    buff.append("HTTP/1.1 " + std::to_string(m_code) + ' ' + status + CRLF);
}

void HttpResponse::addHeaders(Buffer& buff) {
    buff.append("Connection: ");

    if (m_isKeepAlive) {
        buff.append(std::string("Keep-Alive") + CRLF);
        buff.append("Keep-Alive: timeout=" + std::to_string(KEEP_ALIVE_TIMEOUT) + ", max=" + std::to_string(KEEP_ALIVE_MAX) + CRLF);
    }else
        buff.append(std::string("close") + CRLF);

    buff.append(std::string("Content-Type: ") + getFileType() + CRLF);
}

void HttpResponse::addContent(Buffer& buff) {
    int fileFd = open((m_srcDir + m_path).data(), O_RDONLY);

    if (fileFd < 0) {
        replaceWithErrorContent(buff, "File not found");
        return;
    }

    std::string msg = "load file path: " + m_srcDir + m_path;
    Logger::Instance()->LOG_DEBUG(msg);

    // 将资源文件进行内存映射
    int* mmRet = (int*)mmap(nullptr, m_fileState.st_size, PROT_READ, MAP_PRIVATE, fileFd, 0);
    if (*mmRet == -1) {
        replaceWithErrorContent(buff, "File not found");
        return;
    }

    m_memoryMappingFile = (char*)mmRet;
    close(fileFd);

    buff.append("Content-length: " + std::to_string(m_fileState.st_size) + CRLF + CRLF);
}

const std::string HttpResponse::getFileType() const {
    std::string::size_type index = m_path.find_last_of('.');

    if (index != std::string::npos) {
        std::string suffix = m_path.substr(index);
        if (SUFFIX_TYPE.count(suffix) == 1) {
            return SUFFIX_TYPE.find(suffix)->second;
        }
    }

    return "text/plain";
}

void HttpResponse::replaceWithErrorContent(Buffer& buff, std::string msg) const {
    std::string body = "";

    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    body += "<p>" + msg + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    buff.append("Content-length: " + std::to_string(body.size()) + CRLF + CRLF);
    buff.append(body);
}

char* HttpResponse::mmFile() const {
    return m_memoryMappingFile;
}

size_t HttpResponse::mmFileSize() const {
    return m_fileState.st_size;
}

void HttpResponse::unmapFile() {
    if (m_memoryMappingFile) {
        munmap(m_memoryMappingFile, m_fileState.st_size);
        m_memoryMappingFile = nullptr;
    }
}
