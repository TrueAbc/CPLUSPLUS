//
// Created by TrueAbc on 2021/8/9.
//

#ifndef LARS_TCP_SERVER_H
#define LARS_TCP_SERVER_H

#include <netinet/in.h>
#include "reactor_buf.h"
#include "event_loop.h"
#include "tcp_conn.h"

class tcp_server{
    int _sockfd;
    struct sockaddr_in _connaddr; // 客户端连接地址
    socklen_t _addrlen; //  客户端连接地址长度

    event_loop* _loop;

    public:
        tcp_server(event_loop* loop, const char*ip, uint16_t port);

        void do_accept();

        ~tcp_server();

public:
    //  客户端的链接管理
    static void increase_conn(int connfd, tcp_conn *conn);
    static void decrease_conn(int connfd);
    static void get_conn_num(int *curr_conn);
    static tcp_conn **conns;

private:
    //todo 从配置文件读取
#define MAX_CONNS 2
    static int _max_conns; // 最大链接数量

    static int _curr_conns;
    static pthread_mutex_t  _conns_mutex; // 保护修改的锁
};

#endif //LARS_TCP_SERVER_H
