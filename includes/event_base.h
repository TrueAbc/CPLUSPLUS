//
// Created by TrueAbc on 2021/8/18.
//

#ifndef LARS_EVENT_BASE_H
#define LARS_EVENT_BASE_H

class event_loop;

typedef void io_callback(event_loop *loop, int fd, void *args);

struct io_event{
    io_event():read_callback(nullptr), write_callback(nullptr), rcb_args(nullptr), wcb_args(nullptr){};

    int mask; //EPOLLIN EPOLLOUT
    io_callback *read_callback; // EPOLLIN事件触发的回调
    io_callback *write_callback; //EPOLLOUT
    void *rcb_args;
    void *wcb_args;
};

#endif //LARS_EVENT_BASE_H
