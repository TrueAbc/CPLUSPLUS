//
// Created by TrueAbc on 2021/8/19.
//

#ifndef LARS_TCP_CONN_H
#define LARS_TCP_CONN_H
#include "reactor_buf.h"
#include "event_loop.h"
#include "net_connection.h"

class tcp_server;

class tcp_conn: public net_connection{
public:
    tcp_conn(int connfd, event_loop* loop);

    // 读写的kqueue事件触发
    void do_read();
    void do_write();

    void clean_conn();

    // 将信息进行打包
    int send_message(const char *data, int msglen, int msgid);

private:
    int _connfd;
    event_loop *_loop;
    out_buf obuf;   // 写入数据
    input_buf ibuf; // 读取数据
};

#endif //LARS_TCP_CONN_H
