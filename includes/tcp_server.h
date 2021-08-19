//
// Created by TrueAbc on 2021/8/9.
//

#ifndef LARS_TCP_SERVER_H
#define LARS_TCP_SERVER_H

#include <netinet/in.h>
#include "reactor_buf.h"
#include "event_loop.h"

class tcp_server{
    int _sockfd;
    struct sockaddr_in _connaddr; // 客户端连接地址
    socklen_t _addrlen; //  客户端连接地址长度

    event_loop* _loop;

    public:
        tcp_server(event_loop* loop, const char*ip, uint16_t port);

        void do_accept();

        ~tcp_server();
};

#endif //LARS_TCP_SERVER_H
