#ifndef _HTTP_RESPONSE_H
#define _HTTP_RESPONSE_H

#include <string>
#include <unordered_map>
#include <sys/mman.h>   // mmap
#include <fcntl.h>      // open

#include "../buffer/buffer.h"
#include "../logger/logger.h"

#define CRLF                "\r\n"
#define KEEP_ALIVE_TIMEOUT  60
#define KEEP_ALIVE_MAX      16


class HttpResponse {
public:
    HttpResponse() = default;
    ~HttpResponse();

    void init(std::string srcDir, std::string path, bool isKeepAlive, int code);
    void makeResponse(Buffer& buff);

    char* mmFile() const;
    size_t mmFileSize() const;
    void unmapFile();

private:
    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_ERR_PATH;

private:
    int m_code;
    std::string m_srcDir;
    std::string m_path;
    bool m_isKeepAlive;

    char* m_memoryMappingFile;
    struct stat m_fileState;

    void addStatusLine(Buffer& buff);
    void addHeaders(Buffer& buff);
    void addContent(Buffer& buff);

    void replaceWithErrorContent(Buffer& buff, std::string msg) const;

    const std::string getFileType() const;
};

#endif  // _HTTP_RESPONSE_H