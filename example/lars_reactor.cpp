//
// Created by TrueAbc on 2021/8/9.
//

#include "tcp_server.h"

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

int main(){
    event_loop loop;
    tcp_server server(&loop, "127.0.0.1", 7777);
    server.add_msg_router(1, callback_busi);
    server.add_msg_router(2, print_busi);

    loop.event_process();

    return 0;
}