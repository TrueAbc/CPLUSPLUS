//
// Created by TrueAbc on 2021/8/9.
//

#ifndef LARS_TCP_SERVER_H
#define LARS_TCP_SERVER_H

#include <netinet/in.h>
#include "reactor_buf.h"
#include "event_loop.h"
#include "tcp_conn.h"
#include "message.h"
#include "thread_pool.h"

class tcp_server{
    int _sockfd;
    struct sockaddr_in _connaddr; // 客户端连接地址
    socklen_t _addrlen; //  客户端连接地址长度

    event_loop* _loop;

    public:
        tcp_server(event_loop* loop, const char*ip, uint16_t port, int thread_num=5, int max_conns=100);

        void do_accept();

        ~tcp_server();

        thread_pool *get_thread_pool(){
            return _thread_pool;
        }

public:
    //  客户端的链接管理
    static void increase_conn(int connfd, tcp_conn *conn);
    static void decrease_conn(int connfd);
    static void get_conn_num(int *curr_conn);
    static tcp_conn **conns;

    static msg_router router;

    void add_msg_router(int msgid, msg_callback* cb, void *user_data= nullptr){
        router.register_msg_router(msgid, cb, user_data);
    }

private:
    //todo 从配置文件读取
    static int _max_conns; // 最大链接数量

    static int _curr_conns;
    static pthread_mutex_t  _conns_mutex; // 保护修改的锁

public:
    // 创建和销毁链接的回调函数
    static void set_conn_start(conn_callback cb, void *args= nullptr){
        conn_start_cb = cb;
        conn_start_cb_args = args;
    }

    static void set_conn_close(conn_callback cb, void *args= nullptr){
        conn_close_cb = cb;
        conn_close_cb_args = args;
    }

    static conn_callback conn_start_cb;
    static void *conn_start_cb_args;

    static conn_callback conn_close_cb;
    static void *conn_close_cb_args;

private:
    thread_pool *_thread_pool;
};

#endif //LARS_TCP_SERVER_H
