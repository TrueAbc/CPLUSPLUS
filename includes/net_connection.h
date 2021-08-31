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
#endif //LARS_NET_CONNECTION_H
