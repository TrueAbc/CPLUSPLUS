//
// Created by TrueAbc on 2021/9/4.
//

#ifndef LARS_UDP_CLIENT_H
#define LARS_UDP_CLIENT_H

#include "message.h"
#include "net_connection.h"
#include "event_loop.h"

class udp_client: public net_connection{
private:
    int _sockfd;
    char _read_buf[MESSAGE_LENGTH_LIMIT];
    char _write_buf[MESSAGE_LENGTH_LIMIT];

    event_loop *_loop;
    msg_router _router;

public:
    udp_client(event_loop *loop, const char *ip, uint16_t port);
    ~udp_client();

    void add_msg_router(int msgid, msg_callback* cb, void* user_data= nullptr);

    virtual int send_message(const char *data, int msglen, int msgid);

    void do_read();
};

#endif
