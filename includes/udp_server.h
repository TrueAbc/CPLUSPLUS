//
// Created by TrueAbc on 2021/9/4.
//

#ifndef LARS_SERVER_H
#define LARS_SERVER_H

#include <netinet/in.h>
#include "message.h"
#include "net_connection.h"
#include "event_loop.h"

class udp_server: public net_connection{
public:
    udp_server(event_loop *loop, const char *ip, uint16_t port);

    virtual int send_message(const char *data, int datalen, int msgid);

    ~udp_server();

    void do_read();

    void add_msg_router(int msgid, msg_callback* cb, void *user_data= nullptr);

private:
    int _sockfd;

    char _read_buf[MESSAGE_LENGTH_LIMIT];
    char _write_buf[MESSAGE_LENGTH_LIMIT];

    event_loop* _loop;

    // 服务端ip
    struct sockaddr_in _client_addr;
    socklen_t _client_addrlen;

    // 消息路由分发
    msg_router _router;
};
#endif
