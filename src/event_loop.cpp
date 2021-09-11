//
// Created by TrueAbc on 2021/8/18.
//

#include "event_loop.h"
#include "assert.h"


void event_loop::add_task(task_func func, void *args) {
    task_func_pair funcPair(func, args);
    _ready_tasks.push_back(funcPair);
}

void event_loop::execute_ready_tasks() {
    std::vector<task_func_pair>::iterator it;
    for(it = _ready_tasks.begin(); it != _ready_tasks.end(); it++){
        task_func func = it->first;
        void *args = it->second;

        // 执行任务
        func(this, args);
    }
    // 全部执行完毕, 清空当前的_ready_tasks
    _ready_tasks.clear();
}

#ifndef __linux__
event_loop::event_loop() {
  _epfd = kqueue();
  if(_epfd == -1){
      fprintf(stderr, "kqueue create error\n");
      exit(1);
  }
}

void event_loop::event_process() {
    while (true){
        io_event_map_it  ev_it;
        int nfds = kevent(_epfd, nullptr, 0, _fired_evs, MAXEVENTS, nullptr);
        for (int i = 0; i < nfds; ++i) {
            int fd = (int)(intptr_t )_fired_evs[i].udata;
            ev_it = _io_evs.find(fd);
            assert(ev_it != _io_evs.end());

            io_event* ev = &(ev_it->second);
            if(_fired_evs[i].filter == EVFILT_READ && (ev->mask & kReadEvent)){
                //  读取事件
                void *args=ev->rcb_args;
                ev->read_callback(this, fd, args);
            }else if(_fired_evs[i].filter == EVFILT_WRITE && (ev->mask & kWriteEvent)){
                void *args = ev->wcb_args;
                ev->write_callback(this, fd, args);
            }else if(_fired_evs[i].flags & EV_EOF) {
                fprintf(stderr, "error for kqueue, not sure about this");
                this->del_io_event(fd);
            }
        }

        // 每次处理一组epoll_wait触发的事件之后, 处理异步任务
        this->execute_ready_tasks();
    }
}

// 添加对应的事件回调, 注意kqueue的读写是分离的
// 优先添加读事件处理逻辑
void event_loop::add_io_event(int fd, io_callback *proc, int mask, void *args) {
    struct kevent ev[2];
    int n = 0;
    int final_mask;

    int op = EV_ADD | EV_ENABLE; // 如果
    // 多次的操作是没有影响的, 而epoll需要设置是否是modify

    if(mask & kReadEvent){
        EV_SET(&ev[n++], fd, EVFILT_READ, op, 0, 0, (void *)(intptr_t)fd);
    }else if(mask & kWriteEvent){
        EV_SET(&ev[n++], fd, EVFILT_WRITE, op, 0, 0, (void *)(intptr_t)fd);
    }

    int r = kevent(_epfd, ev, n, nullptr,0 , nullptr);
    // 在kqueue中添加, 下面是在结构体内部添加回调函数
    io_event_map_it  it = _io_evs.find(fd);
    if(it == _io_evs.end()){
        final_mask = mask;
    }else{
        final_mask = it->second.mask | mask;
    }
    if(mask & kReadEvent){
        _io_evs[fd].read_callback = proc;
        _io_evs[fd].rcb_args = args;
    }else if(mask & kWriteEvent){
        _io_evs[fd].write_callback = proc;
        _io_evs[fd].wcb_args = args;
    }
    _io_evs[fd].mask = final_mask;

    listen_fds.insert(fd);
    if( r ){
        fprintf(stderr, "error for add event");
        exit(1);
    }
}

void event_loop::del_io_event(int fd) {
    _io_evs.erase(fd);

    listen_fds.erase(fd);
    // 删除 堆中的信息
    struct kevent ev[2];
    int n = 0;

    EV_SET(&ev[n++], fd, EVFILT_READ, EV_DELETE, 0, 0, (void*)(intptr_t)fd);
    EV_SET(&ev[n++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, (void*)(intptr_t)fd);

    kevent(_epfd, ev, n, nullptr, 0, nullptr);
    //todo 针对没有添加的event会报错, 这里暂时不检测返回值,
    //    if (r ){
//        fprintf(stderr, "error for delete a fd\n");
//        exit(1);
//    }
}

void event_loop::del_io_event(int fd, int mask) {
    io_event_map_it  it = _io_evs.find(fd);
    if(it == _io_evs.end()){
        return;
    }

    int &o_mask = it->second.mask;
    o_mask = o_mask & (~mask);

    int n = 0;
    if (o_mask == 0){
        this->del_io_event(fd);
    }else{
        // 修正本地的回调用事件以及修正堆中的事件
        struct kevent ev[2];
        if(o_mask == kReadEvent){
            EV_SET(&ev[n++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, (void*)(intptr_t)fd);
            it->second.write_callback = nullptr;
        } else{
            EV_SET(&ev[n++], fd, EVFILT_READ, EV_DELETE, 0, 0, (void*)(intptr_t)fd);
            it->second.read_callback = nullptr;
        }
        int r = kevent(_epfd, ev, n, nullptr, 0, nullptr);
        if (r ){
            fprintf(stderr, "error for delete a fd\n");
            exit(1);
        }
    }
}

#else
// epoll可用的情况处理

event_loop::event_loop(){
    // wsl的create依旧需要配置size选项
    _epfd = epoll_create(10);
    if(_epfd == -1){
        fprintf(stderr, "epoll create error\n");
        exit(1);
    }
}

void event_loop::event_process(){
    while(true){
        io_event_map_it ev_it;

        int nfds = epoll_wait(_epfd, _fired_evs, MAXEVENTS, 10);
        for(int i=0;i<nfds;i++){
            ev_it = _io_evs.find(_fired_evs[i].data.fd);

            assert(ev_it != _io_evs.end());

            io_event *ev = &(ev_it->second);

            if(_fired_evs[i].events & kReadEvent){
                void *args = ev->rcb_args;
                ev->read_callback(this, _fired_evs[i].data.fd, args);
            }else if(_fired_evs[i].events & kWriteEvent){
                void *args = ev->wcb_args;
                ev->write_callback(this, _fired_evs[i].data.fd, args);
            }else if(_fired_evs[i].events & (EPOLLHUP | EPOLLERR)){
                if(ev->read_callback != nullptr){
                    void *args = ev->rcb_args;
                    ev->read_callback(this, _fired_evs[i].data.fd, args);
                }else if(ev->write_callback != nullptr){
                    void *args = ev->wcb_args;
                    ev->write_callback(this, _fired_evs[i].data.fd, args);
                }else{
                    fprintf(stderr, "fd %d  get error, delete from epoll\n", _fired_evs[i].data.fd);
                    this->del_io_event(_fired_evs[i].data.fd);
                }
            }
        }
    }
}

void event_loop::add_io_event(int fd, io_callback* proc, int mask, void *args){
    int final_mask;

    int op;

    // 1. 找到当前fd是否已经有事件
    io_event_map_it it = _io_evs.find(fd);
    if(it == _io_evs.end()){
        //2. 没有找到就是add
        final_mask = mask;
        op = EPOLL_CTL_ADD;
    }else{
        // 3. 如果有就是mod
        final_mask = it->second.mask | mask;
        op = EPOLL_CTL_MOD;
    }

    // 4. 注册回调函数
    if(mask & kReadEvent){
        _io_evs[fd].read_callback = proc;
        _io_evs[fd].rcb_args = args;
    }else if(mask & kWriteEvent){
        _io_evs[fd].write_callback = proc;
        _io_evs[fd].wcb_args = args;
    }

    //5. 添加到epoll堆中
    _io_evs[fd].mask = final_mask;
    struct epoll_event event;
    event.events = final_mask;
    event.data.fd = fd;

    if(epoll_ctl(_epfd, op, fd, &event) == -1){
        fprintf(stderr, "epoll ctl %d error\n", fd);
        return;
    }

    listen_fds.insert(fd);
}

void event_loop::del_io_event(int fd){
    _io_evs.erase(fd);
    listen_fds.erase(fd);

    epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, nullptr);
}

void event_loop::del_io_event(int fd, int mask){
    io_event_map_it it = _io_evs.find(fd);
    if(it == _io_evs.end()){
        return;
    }

    int &o_mask = it->second.mask;
    o_mask = o_mask &(~mask);

    if(o_mask == 0){
        this->del_io_event(fd);
    }else{
        struct epoll_event event;
        event.events = o_mask;
        event.data.fd = fd;
        epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &event);
        
    }
}

#endif