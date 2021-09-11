//
// Created by TrueAbc on 2021/8/11.
//

#ifndef LARS_REACTOR_BUF_H
#define LARS_REACTOR_BUF_H

#include "io_buf.h"
#include "buf_pool.h"
#include <cassert>
#include <unistd.h>

class reactor_buf{
public:
    reactor_buf();
    ~reactor_buf();

    const int length() const;
    void pop(int len);
    void clear();

protected:
    io_buf *_buf;
};

//  读取缓存buffer
class input_buf: public reactor_buf{
public:
    int read_data(int fd);

    const char*data() const; // 取出读到的数据

    void adjust(); // 重置缓冲区
};

class out_buf: public reactor_buf{
public:
    int send_data(const char* data, int datalen);

    int write2fd(int fd);
};

#endif //LARS_REACTOR_BUF_H

