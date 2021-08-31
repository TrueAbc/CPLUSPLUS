//
// Created by TrueAbc on 2021/8/19.
//

#ifndef LARS_MESSAGE_H
#define LARS_MESSAGE_H

#include "net_connection.h"

struct msg_head{
    int32_t msgid;
    int32_t msglen;
};

// 消息头的二进制长度, 固定值
#define MESSAGE_HEAD_LEN 8
#define MESSAGE_LENGTH_LIMIT (65535 - MESSAGE_HEAD_LEN)

typedef void msg_callback(const char*data, uint32_t len, int msgid,net_connection  *conn, void *userdata);

// 消息分发机制
class msg_router{
public:
    msg_router():_router(),_args(){
        printf("msg router init...\n");
    }

    int register_msg_router(int msgid, msg_callback *msg_cb, void *user_data){
        if(_router.find(msgid) != _router.end()){
            return -1;
        }

        printf("add msg cb msgid=%d\n", msgid);
        _router[msgid] = msg_cb;
        _args[msgid] = user_data;
        return 0;
    }

    void call(int msgid, uint32_t msglen, const char *data,net_connection* client){
        printf("call msgid= %d\n", msgid);
        if(_router.find(msgid) == _router.end()){
            fprintf(stderr, "msgid %d is not register\n", msgid);
            return;
        }

        msg_callback  *callback = _router[msgid];
        void *user_data = _args[msgid];
        callback(data, msglen, msgid, client, user_data);
        printf("=====\n");
    }

private:
    // 针对消息的路由分发, key 为msgID, value为注册的回调业务函数
    __gnu_cxx::hash_map<int, msg_callback*>_router;

    // 回调业务对应的参数
    __gnu_cxx::hash_map<int, void*> _args;
};
#endif //LARS_MESSAGE_H
