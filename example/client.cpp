//
// Created by TrueAbc on 2021/8/30.
//

#include "tcp_client.h"
#include <cstdio>

void busi(const char*data, uint32_t len, int msgid, net_connection *conn, void *userdata){
    printf("\nrecv server:[%s]\n", data);
    printf("msgid:[%d]\n", msgid);
    printf("len:[%d], strlen(%d)\n\n", len, strlen(data));
}

void on_client_build(net_connection* conn, void* args){
    int msgid=1;
    const char *msg="Hello Lars!";
    conn->send_message(msg, strlen(msg), msgid);
}

void on_client_lost(net_connection *conn, void *args){
    printf("on client lost ...\n");
    printf("client is lost!\n");
}

int main(){
    event_loop loop;
    tcp_client client(&loop, "127.0.0.1", 8888, "clientv0.4");

    client.add_msg_router(1, busi);
    client.add_msg_router(101, busi);

    client.set_conn_cb(on_client_build);
    client.set_conn_cb(on_client_lost, nullptr, 1);

    const char *data = "test from client";
    client.send_message(data, strlen(data), 1);
    loop.event_process();
    return 0;
}
