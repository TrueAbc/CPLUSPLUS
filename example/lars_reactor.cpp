//
// Created by TrueAbc on 2021/8/9.
//

#include "tcp_server.h"
#include "config_file.h"

tcp_server *server;
pthread_mutex_t t;

void print_lars_task(event_loop *loop, void *args){
    printf("----------Active Task Func!----------\n");
    io_event_set fds;

    loop->get_listen_fds(fds); // 不同线程的loop, 返回的fds不同
    io_event_set::iterator it;
    for(it = fds.begin();it != fds.end() ;it++){
        int fd = *it;
        tcp_conn *conn = tcp_server::conns[fd];
        if(conn != nullptr){
            int msgid = 101;
            const char *msg = "Hello I am a Task!";
            // send涉及到内部的数据保护

            conn->send_message(msg, strlen(msg), msgid);

        }
    }
}

// 回显业务的回调函数
void callback_busi(const char *data, uint32_t len, int msgid, net_connection *conn, void *user_data){
    printf("callback busi...\n");
    conn->send_message(data, len, msgid);
}
// 打印信息回调函数
void print_busi(const char *data, uint32_t len, int msgid, net_connection *conn, void *user_data){
    printf("\nrecv client:[%s]\n", data);
    printf("msgid:[%d]\n", msgid);
    printf("len:[%d]\n\n", len);
}

// 创建和销毁的回调函数
void on_client_build(net_connection *conn, void *args){
    int msgid=101;
    const char *msg = "welcome! you on line---------";
    conn->send_message(msg, strlen(msg), msgid);

    server->get_thread_pool()->send_task(print_lars_task);
}

void on_client_lost(net_connection* conn, void *args){
    printf("connection is lost\n");
}

int main(){
    event_loop loop;

    pthread_mutex_init(&t, nullptr);
    config_file::setPath("./server.conf");
    std::string ip = config_file::instance()->GetString("reactor", "ip", "0.0.0.0");
    short port = config_file::instance()->GetNumber("reactor", "port", 8888);

    printf("ip = %s, port = %d\n", ip.c_str(), port);

    server = new tcp_server(&loop, ip.c_str(), port);
    

    server->add_msg_router(1, callback_busi);
    server->add_msg_router(2, print_busi);

    server->set_conn_start(on_client_build);
    server->set_conn_close(on_client_lost);

    loop.event_process();

    return 0;
}