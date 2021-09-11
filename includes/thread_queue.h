//
// Created by TrueAbc on 2021/8/31.
//

#ifndef LARS_THREAD_QUEUE_H
#define LARS_THREAD_QUEUE_H
#include <queue>
#include <pthread.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>
#include "event_loop.h"


template<typename T>
class thread_queue{
public:
    thread_queue(){
        // 原文使用eventfd, 这里使用mac 的pipe
        _loop = nullptr;
        pthread_mutex_init(&_queue_mutex, nullptr);

        int ret = pipe(_pipe);
        if(ret < 0){
            perror("pipe init error");
            exit(1);
        }
        // 读接口非阻塞
        int flags = fcntl(_pipe[0], F_GETFL, 0);
        fcntl(_pipe[0], F_SETFL, flags | O_NONBLOCK);

    }

    ~thread_queue(){
        pthread_mutex_destroy(&_queue_mutex);
        close(_evfd);
    }

    // 向队列添加任务
    void send(const T& task){
        // 触发消息事件的占位符
        unsigned long long idle_num = 1;
        pthread_mutex_lock(&_queue_mutex);
        _queue.emplace(task);

        // 触发_evfd的read事件, 处理该任务
        int ret = write(_pipe[1], &idle_num, sizeof (unsigned  long long));
        if(ret == -1){
            perror("_evfd write");
        }

//        printf("write message ok\n");
        pthread_mutex_unlock(&_queue_mutex);
    }

    // 获取队列
    void recv(std::queue<T>& new_queue){
        unsigned  int long long idle_num = 1;
        pthread_mutex_lock(&_queue_mutex);
        // mac下暂时使用管道
        // eventfd的特性, 所有的write会在一次的read后累加出来
        int ret = 0;
        do{
            ret = read(_pipe[0], &idle_num, sizeof (unsigned long long));
        } while (ret >= 0);
        //-1 代表无数据可以读取
        // 把占位符的内容读取出来, 确保缓冲区没有数据存

        std::swap(new_queue, _queue);
        pthread_mutex_unlock(&_queue_mutex);
    }

    void set_loop(event_loop *loop){
        _loop = loop;
    }

    void set_callback(io_callback *cb, void* args= nullptr){
        if(_loop != nullptr){
            _loop->add_io_event(_pipe[0], cb, kReadEvent, args);
        }
    }

    event_loop *get_loop(){
        return _loop;
    }

private:
    int _pipe[2];
    int _evfd; //触发消息读取的业务的fd
    event_loop *_loop; // 当前消息队列绑定的event_loop事件
    std::queue<T> _queue; // 任务队列
    pthread_mutex_t _queue_mutex; // 保护锁
};

#endif //LARS_THREAD_QUEUE_H
