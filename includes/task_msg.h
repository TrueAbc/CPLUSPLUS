//
// Created by TrueAbc on 2021/8/31.
//

#ifndef LARS_TASK_MSG_H
#define LARS_TASK_MSG_H

#include "event_loop.h"
// 异步调用的函数类型
typedef void (*task_func) (event_loop *loop, void *args);

struct task_msg {
    enum TASK_TYPE{
        NEW_CONN, // 新建链接的任务
        NEW_TASK, // 响应的任务
    };

    TASK_TYPE type;

    union {
        int connfd; // conn任务需要传递connfd

        // 针对task, 需要传递回调函数
        struct {
            task_func task_cb;
            void *args;
        };

    };
};

#endif //LARS_TASK_MSG_H
