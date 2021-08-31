//
// Created by TrueAbc on 2021/8/31.
//

#ifndef LARS_NET_CONNECTION_H
#define LARS_NET_CONNECTION_H

/*
 * 网络通信的抽象类, 任何需要收发消息的模块, 都可以进行实现
 */
class net_connection{
public:
    virtual int send_message(const char *data, int datalen, int msgid)=0;
};

// 创建链接/ 销毁链接触发的回调函数

typedef void (*conn_callback)(net_connection *conn, void *args);
#endif //LARS_NET_CONNECTION_H
