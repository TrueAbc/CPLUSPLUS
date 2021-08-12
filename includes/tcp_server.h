//
// Created by TrueAbc on 2021/8/9.
//

#ifndef LARS_TCP_SERVER_H
#define LARS_TCP_SERVER_H

#include <netinet/in.h>
#include "reactor_buf.h"

class tcp_server{
    int _sockfd;
    struct sockaddr_in _connaddr; // 客户端连接地址
    socklen_t _addrlen; //  客户端连接地址长度

    public:
        tcp_server(const char*ip, uint16_t port);

        void do_accept();

        ~tcp_server();
};

#endif //LARS_TCP_SERVER_H
