//
// Created by TrueAbc on 2021/8/18.
//

#ifndef LARS_EVENT_LOOP_H
#define LARS_EVENT_LOOP_H

#ifndef __linux__
#include <sys/event.h>
const int kReadEvent = 1;
const int kWriteEvent = 2
#else
#include <sys/epoll.h>
const int kReadEvent = EPOLLIN;
const int kWriteEvent = EPOLLOUT;
#endif

#include <ext/hash_map>
#include <ext/hash_set>
#include <vector>
#include "event_base.h"

#include "task_msg.h"

#define MAXEVENTS 10
typedef __gnu_cxx::hash_map<int, io_event> io_event_map; // fd->io_event
typedef __gnu_cxx::hash_map<int, io_event>::iterator io_event_map_it;

typedef __gnu_cxx::hash_set<int> io_event_set;

typedef void (*task_func)(event_loop *loop, void *args);


class event_loop{
public:
    event_loop(); //构建epoll堆

    void event_process(); // 永久阻塞, 等待触发的事件调用对应的callback方法

    void add_io_event(int fd, io_callback* proc, int mask, void *args= nullptr);
    void del_io_event(int fd);
    void del_io_event(int fd, int mask); // epollin / epollout需要转化为kqueue

    // 获取全部监听事件的fd集合
    void get_listen_fds(io_event_set &fds){
        fds = listen_fds;
    }

    // 异步任务task需要添加的方法
    void add_task(task_func func, void *args);
    void execute_ready_tasks(); // 执行全部的ready_tasks里面的任务

private:
    int _epfd;
    io_event_map _io_evs; // fd和对应的事件之间的关系
    io_event_set listen_fds;

    #ifndef __linux__
    struct kevent _fired_evs[MAXEVENTS]; // 需要处理的event

    #else
    struct epoll_event _fired_evs[MAXEVENTS];
    #endif

    typedef std::pair<task_func, void*> task_func_pair;
    // 需要被执行的task集合
    std::vector<task_func_pair> _ready_tasks;
};
#endif //LARS_EVENT_LOOP_H
