//
// Created by TrueAbc on 2021/8/30.
//

#include "tcp_client.h"
#include <cstdio>

void busi(const char*data, uint32_t len, int msgid, tcp_client *conn, void *userdata){
    printf("\nrecv server:[%s]\n", data);
    printf("msgid:[%d]\n", msgid);
    printf("len:[%d]\n\n", len);
}

int main(){
    event_loop loop;
    tcp_client client(&loop, "127.0.0.1", 7777, "clientv0.4");

    client.set_msg_callback(busi);
    loop.event_process();
    return 0;
}
