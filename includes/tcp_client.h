//
// Created by TrueAbc on 2021/8/30.
//

#ifndef LARS_TCP_CLIENT_H
#define LARS_TCP_CLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "buf_pool.h"
#include "event_loop.h"
#include "message.h"

class tcp_client;

typedef void msg_callback(const char*data, uint32_t len, int msgid, tcp_client *conn, void *userdata);

class tcp_client{
public:
    tcp_client(event_loop *loop, const char *ip, unsigned  short port, const char *name);

    int send_message(const char *data, int msglen, int msgid);// 发送message

    void do_connect(); // 创建链接

    int do_read();

    int do_write();

    void clean_conn();

    ~tcp_client();

    void set_msg_callback(msg_callback *msg_cb){
        this->_msg_callback = msg_cb;
    }

    bool connected;// 链接是否创建成功
    struct sockaddr_in _server_addr;
    io_buf _obuf;
    io_buf _ibuf;

private:
    int _sockfd;
    socklen_t _addrlen;

    event_loop* _loop; // 客户端的事件处理机制

    const char * _name; //  用于记录日志的客户端名称

    msg_callback* _msg_callback;
};
#endif //LARS_TCP_CLIENT_H
