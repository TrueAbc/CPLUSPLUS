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
#include "net_connection.h"

class tcp_client: public net_connection{
public:
    tcp_client(event_loop *loop, const char *ip, unsigned  short port, const char *name);

    int send_message(const char *data, int msglen, int msgid);// 发送message

    void do_connect(); // 创建链接

    int do_read();

    int do_write();

    void clean_conn();

    ~tcp_client();

   void add_msg_router(int msgid, msg_callback* msg_cb, void *user_data= nullptr){
       router.register_msg_router(msgid, msg_cb, user_data);
   }

   void set_conn_cb(conn_callback cb, void *args= nullptr, int flag=0){
       switch (flag) {
           case 0:
               _conn_start_cb = cb;
               _conn_start_cb_args = args;
               break;
           case 1:
               _conn_close_cb = cb;
               _conn_close_cb_args = args;
               break;
       };
   }

   conn_callback _conn_start_cb;
   void* _conn_start_cb_args;

   conn_callback  _conn_close_cb;
   void * _conn_close_cb_args;

    bool connected;// 链接是否创建成功
    struct sockaddr_in _server_addr;
    io_buf _obuf;
    io_buf _ibuf;

private:
    int _sockfd;
    socklen_t _addrlen;

    event_loop* _loop; // 客户端的事件处理机制

    const char * _name; //  用于记录日志的客户端名称

    msg_router router;
};
#endif //LARS_TCP_CLIENT_H
