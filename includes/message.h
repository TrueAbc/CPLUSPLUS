//
// Created by TrueAbc on 2021/8/19.
//

#ifndef LARS_MESSAGE_H
#define LARS_MESSAGE_H

struct msg_head{
    int32_t msgid;
    int32_t msglen;
};

// 消息头的二进制长度, 固定值
#define MESSAGE_HEAD_LEN 8
#define MESSAGE_LENGTH_LIMIT (65535 - MESSAGE_HEAD_LEN)


#endif //LARS_MESSAGE_H
