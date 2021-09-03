//
// Created by TrueAbc on 2021/8/9.
//

#include "tcp_server.h"
#include "config_file.h"

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
}

void on_client_lost(net_connection* conn, void *args){
    printf("connection is lost\n");
}

int main(){
    event_loop loop;

    config_file::setPath("./server.conf");
    std::string ip = config_file::instance()->GetString("reactor", "ip", "0.0.0.0");
    short port = config_file::instance()->GetNumber("reactor", "port", 8888);

    printf("ip = %s, port = %d\n", ip.c_str(), port);

    tcp_server server(&loop, ip.c_str(), port);
    

    server.add_msg_router(1, callback_busi);
    server.add_msg_router(2, print_busi);

    server.set_conn_start(on_client_build);
    server.set_conn_close(on_client_lost);

    loop.event_process();

    return 0;
}