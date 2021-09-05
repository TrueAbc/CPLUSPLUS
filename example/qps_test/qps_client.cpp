//
// Created by TrueAbc on 2021/9/5.
//
#include "stdlib.h"
#include "tcp_client.h"
#include "echoMessage.pb.h"

#include "time.h"

struct Qps{
    Qps(){
        last_time = time(nullptr);
        succ_cnt = 0;
    }

    long last_time; // 最后一次发包的时间 ms
    int succ_cnt; // 成功收到的数量
};

void busi(const char *data, uint32_t len, int msgid, net_connection* conn, void * user_data){
    Qps *qps = (Qps*)user_data;

    qps_test::EchoMessage req, res;
    if(res.ParseFromArray(data, len) == false){
        printf("server call back data error\n");
        return;
    }

    if(res.content() == "Hello Lars!!!"){
        qps->succ_cnt++;
    }

    long cur = time(nullptr);
    if(cur - qps->last_time >= 1){
        printf("---->qps=%d<----\n", qps->succ_cnt);
        qps->last_time = cur;
        qps->succ_cnt = 0;
    }

    req.set_id(res.id() + 1);
    req.set_content(res.content());
    std::string reqS;
    req.SerializeToString(&reqS);

    conn->send_message(reqS.c_str(), reqS.size(), msgid);
}

void connection_start(net_connection* client, void *args){
    qps_test::EchoMessage req;
    req.set_id(1);
    req.set_content("Hello Lars!!!");

    std::string reqS;
    req.SerializeToString(&reqS);
    printf("start :%s\n", reqS.c_str());

    int msgid = 1;
    client->send_message(reqS.c_str(), reqS.size(), msgid);
}

void *thread_main(void *args){
    event_loop loop;
    tcp_client client(&loop, "127.0.0.1", 8888, "qps client");

    Qps qps;
    client.add_msg_router(1, busi, (void *)&qps);
    client.set_conn_cb(connection_start);
    client.clean_conn();// 触发启动事件

    loop.event_process();
    return nullptr;
}

int main(int argc, char **argv){
    if(argc == 1){
        printf("Usage: ./client [threadnum]\n");
        return 1;
    }
    int thread_num = atoi(argv[1]);

    pthread_t *tids;
    tids = new pthread_t[thread_num];

    for(int i=0;i<thread_num;i++){
        pthread_create(&tids[i], nullptr, thread_main, nullptr);
    }

    for(int i=0;i<thread_num;i++){
        pthread_join(tids[i], nullptr);
    }
    return 0;
}