//
// Created by TrueAbc on 2021/8/18.
//

#ifndef LARS_EVENT_LOOP_H
#define LARS_EVENT_LOOP_H

#include <sys/event.h>
#include <ext/hash_map>
#include <ext/hash_set>
#include "event_base.h"

#define MAXEVENTS 10
typedef __gnu_cxx::hash_map<int, io_event> io_event_map; // fd->io_event
typedef __gnu_cxx::hash_map<int, io_event>::iterator io_event_map_it;

typedef __gnu_cxx::hash_set<int> io_event_set;

const int kReadEvent = 1;
const int kWriteEvent = 2;

class event_loop{
public:
    event_loop(); //构建epoll堆

    void event_process(); // 永久阻塞, 等待触发的事件调用对应的callback方法

    void add_io_event(int fd, io_callback* proc, int mask, void *args= nullptr);
    void del_io_event(int fd);
    void del_io_event(int fd, int mask); // epollin / epollout需要转化为kqueue

private:
    int _epfd;
    io_event_map _io_evs; // fd和对应的事件之间的关系
    io_event_set listen_fds;
    struct kevent _fired_evs[MAXEVENTS]; // 需要处理的event
};
#endif //LARS_EVENT_LOOP_H
