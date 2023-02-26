#ifndef _HTTP_CONN_H
#define _HTTP_CONN_H

#include <sys/types.h>

class HttpConn {
public:
    HttpConn();

    ssize_t read();
};

#endif  // _HTTP_CONN_H