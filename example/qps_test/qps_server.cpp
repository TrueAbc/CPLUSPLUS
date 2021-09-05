//
// Created by TrueAbc on 2021/9/5.
//

#include "config_file.h"
#include "tcp_server.h"
#include "echoMessage.pb.h"

void callback_busi(const char *data, uint32_t len, int msgid, net_connection* conn, void *user_data){
    qps_test::EchoMessage request, responese;

    request.ParseFromArray(data, len);

    responese.set_id(request.id());
    responese.set_content(request.content());

    std::string responseString;
    responese.SerializeToString(&responseString);

    conn->send_message(responseString.c_str(), responseString.size(), msgid);
}

int main(){
    event_loop loop;

    config_file::setPath("../server.conf");
    std::string ip = config_file::instance()->GetString("reactor", "ip", "0.0.0.0");
    short port = config_file::instance()->GetNumber("reactor", "port", 8888);

    printf("ip = %s, port = %d\n", ip.c_str(), port);

    tcp_server server(&loop, ip.c_str(), port);

    //注册消息业务路由
    server.add_msg_router(1, callback_busi);

    loop.event_process();

    return 0;
}